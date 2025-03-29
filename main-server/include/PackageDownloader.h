//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#define KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#include "HeavyJson.h"
#include "LightJson.h"
#include <folly/MPMCQueue.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <cpprest/http_client.h>
#include <string>
#include <thread>

namespace main_server{
    
using request_sender = std::function<pplx::task<web::http::http_response>(
    const std::string& url,
    const web::http::http_request&
)>;

using database_storer = std::function<void(const HeavyJSON&)>;

struct Repo {
    std::string name;
    std::string subnet_url;
    std::vector<std::string> proxy_urls;
};

class PackageDownloader {
public:
    PackageDownloader(folly::MPMCQueue<LightJSON> &download_queue,
        folly::MPMCQueue<HeavyJSON> &output_queue, std::string repos_config_file = "repos.list");

    void start();
    void stop();

    void process_loop();
    void download_package(const LightJSON& package);

    std::vector<std::string> generate_urls(const LightJSON& package);
    static void store_to_database(const HeavyJSON& package);
    pplx::task<web::http::http_response> send_request(const std::string& url, const web::http::http_request& method);

    void update_repos();
    const std::unordered_map<std::string, Repo>& get_allowed_repos() const;

    database_storer store_to_database_;
    request_sender send_request_;

private:

    folly::MPMCQueue<LightJSON> &download_queue_;
    folly::MPMCQueue<HeavyJSON> &output_queue_;
    std::thread worker_;
    folly::CPUThreadPoolExecutor executor_;
    std::atomic<bool> is_running_{false};

    std::string repos_config_file_;
    std::unordered_map<std::string, Repo> allowed_repositories_;
};

}// namespace main_server


#endif //KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
