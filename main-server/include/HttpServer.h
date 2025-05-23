//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H
#define KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H

#include "HeavyJson.h"
#include "LightJson.h"
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/Task.h>
#include <folly/MPMCQueue.h>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>

namespace main_server {
    class HttpServer {
    public:
        friend class TestableHttpServer;
        explicit HttpServer(const std::string&           url,
                            folly::MPMCQueue<LightJSON>& input_queue,
                            folly::MPMCQueue<HeavyJSON>& output_queue);
        ~HttpServer() = default;

        void start();
        void stop();
        void process_loop();

        HttpServer(const HttpServer&)            = delete;
        HttpServer& operator=(const HttpServer&) = delete;
        HttpServer(HttpServer&&)                 = delete;
        HttpServer& operator=(HttpServer&&)      = delete;

        // private:
        void handle_get_request(const web::http::http_request& request);  // check exist packages
        void handle_post_request(const web::http::http_request& request); // install packages(apt-install)
        bool validate_light_json(const LightJSON& json);

        folly::coro::Task<void> response_request(const HeavyJSON& heavyJson);

        web::http::experimental::listener::http_listener listener;
        std::shared_ptr<folly::CPUThreadPoolExecutor>    executor_;
        folly::MPMCQueue<LightJSON>&                     input_queue_;
        folly::MPMCQueue<HeavyJSON>&                     output_queue_;
        std::atomic<bool>                                is_running_{false};
    };

} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H