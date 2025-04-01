//
// Created by Kymus-team on 2/22/25.
//

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
