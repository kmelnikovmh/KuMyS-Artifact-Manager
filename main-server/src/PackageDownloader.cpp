#ifndef KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#define KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#include "HeavyJson.h"
#include "LightJson.h"
#include <folly/MPMCQueue.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <cpprest/http_client.h>
#include <string>
#include <thread>

namespace main_server {

    using send_request_handler = 
        std::function<web::http::http_response(
            const std::string&, 
            const web::http::method&)>;

    using store_to_database_handler = 
        std::function<folly::coro::Task<void>(const HeavyJSON&)>;

    // temporary; TODO
    struct Repo {
        std::string              name;
        std::string              subnet_url;
        std::vector<std::string> proxy_urls;
    };


    class PackageDownloader {
    public:
    bool is_running() const { return is_running_.load(); }
        PackageDownloader(folly::MPMCQueue<LightJSON>& download_queue,
                          folly::MPMCQueue<HeavyJSON>& output_queue,
                          std::string                  repos_config_file = "../repos.list");

        void start();
        void stop();

        void process_loop();
        folly::coro::Task<void> download_package(LightJSON package);

        std::vector<std::string>             generate_urls(const LightJSON& package);
        void                                         update_repos();
        const std::unordered_map<std::string, Repo>& get_allowed_repos() const;

        store_to_database_handler store_to_database_;
        send_request_handler       send_request_;

    // private:
        folly::MPMCQueue<LightJSON>& download_queue_;
        folly::MPMCQueue<HeavyJSON>& output_queue_;
        std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;
        std::atomic<bool>            is_running_{false};

        std::string                           repos_config_file_;
        std::unordered_map<std::string, Repo> allowed_repositories_;
    };

} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H


//
// Created by Kymus-team on 2/22/25.
//

#include "PackageDownloader.h"
#include "DatabaseManager.h"
#include "HeavyJson.h"
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <cpprest/http_client.h>
#include <cpprest/uri_builder.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <utility>

using namespace main_server;

PackageDownloader::PackageDownloader(folly::MPMCQueue<LightJSON>& download_queue,
                                     folly::MPMCQueue<HeavyJSON>& output_queue,
                                     std::string                  repos_config_file)
    : download_queue_(download_queue)
    , output_queue_(output_queue)
    , executor_(std::make_shared<folly::CPUThreadPoolExecutor>(std::thread::hardware_concurrency()))
    , repos_config_file_(std::move(repos_config_file))
    , store_to_database_([](const HeavyJSON& package) -> folly::coro::Task<void> {
        return DatabaseManager::store_package(package);
    })
    , send_request_([](const std::string& url, const web::http::http_request& method) {
        web::http::client::http_client client(U(url));
        return client.request(method).get();
    }) {
}

void PackageDownloader::start() {
    update_repos();
    is_running_.store(true);
    std::thread([this]() { process_loop(); }).detach();
}

void PackageDownloader::stop() {
    is_running_.store(false);
    executor_->join();
}

void PackageDownloader::process_loop() {
    while (is_running_.load()) {
        LightJSON package;
        download_queue_.blockingRead(package);
        folly::coro::co_invoke([this, pkg = std::move(package)] { return download_package(std::move(pkg)); })
            .scheduleOn(executor_.get())
            .start();
    }
}

folly::coro::Task<void> PackageDownloader::download_package(LightJSON package) {
    const auto urls = generate_urls(package);
    if (urls.empty()) {
        std::cerr << "No available urls to download package " << package.name << std::endl;
        co_return;
    }

    bool is_downloaded = false;
    for (size_t i = 0; i < urls.size() && !is_downloaded; ++i) {
        const auto& url = urls[i];
        try {
            auto response = send_request_(url, web::http::methods::GET);
            
            if (response.status_code() != web::http::status_codes::OK) {
                throw std::runtime_error("HTTP error: " + std::to_string(response.status_code()));
            }

            auto data = response.extract_vector().get();
            HeavyJSON downloaded_package{
                package.id,
                package.request_type,
                package.name,
                package.version,
                package.architecture,
                package.check_sum,
                package.repo,
                package.path,
                data.size(),
                data,
                utility::datetime::utc_now().to_string()
            };
            for (auto&& header : response.headers()) {
                downloaded_package.headers[header.first] = header.second;
            }

            if (downloaded_package.request_type != "update") {
                co_await store_to_database_(downloaded_package);
            }

            output_queue_.blockingWrite(std::move(downloaded_package));
            is_downloaded = true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to download from " << url << ": " << e.what() << std::endl;
        }
    }

    if (!is_downloaded) {
        std::cerr << "Failed to download package " << package.name << std::endl;
    }
}

std::vector<std::string> PackageDownloader::generate_urls(const LightJSON& package) {
    if (!allowed_repositories_.contains(package.repo)) {
        std::cerr << "Repo " << package.repo << " is not listed" << std::endl;
        return {};
    }
    const Repo&              repo = allowed_repositories_[package.repo];
    std::vector<std::string> urls;
    for (const auto& repo_proxy_url : repo.proxy_urls) {
        web::uri_builder builder;

        std::size_t protocol_pos = repo_proxy_url.find("://");
        if (protocol_pos != std::string::npos) {
            std::string scheme = repo_proxy_url.substr(0, protocol_pos);
            std::string host   = repo_proxy_url.substr(protocol_pos + 3);
            builder.set_scheme(utility::conversions::to_string_t(scheme))
                .set_host(utility::conversions::to_string_t(host));
        } else {
            builder.set_scheme(U("http")).set_host(U(repo_proxy_url));
        }

        builder.append_path(U(package.path))
            .append_path(U(package.name + "_" + package.version + "_" + package.architecture + ".deb"));

        urls.push_back(builder.to_string());
    }
    return urls;
}

void PackageDownloader::update_repos() {
    allowed_repositories_.clear();
    std::ifstream file(repos_config_file_);
    std::string   line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        Repo               repo;
        iss >> repo.name;
        std::string url;
        while (iss >> url) {
            repo.proxy_urls.push_back(url);
        }
        allowed_repositories_[repo.name] = repo;
    }
}

const std::unordered_map<std::string, Repo>& PackageDownloader::get_allowed_repos() const {
    return allowed_repositories_;
}
