//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H
#define KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H

#include <cpprest/http_listener.h>
#include <folly/MPMCQueue.h>
#include "LightJson.h"
#include "HeavyJson.h"

namespace main_server{
class HttpServer{
public:
    explicit HttpServer(std::string url,folly::MPMCQueue<LightJSON>& input_queue, folly::MPMCQueue<HeavyJSON>& output_queue );
    ~HttpServer() = default;

    void start();
    void stop();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    HttpServer(HttpServer&&) = delete;
    HttpServer& operator=(HttpServer&&) = delete;
private:
    void handle_get_request(web::http::http_request request); // check exist packages
    void handle_post_request(web::http::http_request request); // install packages(apt-install)
    bool validate_light_json(const LightJSON& json);

    web::http::experimental::listener::http_listener listener;
    folly::MPMCQueue<LightJSON>& input_queue_;
    folly::MPMCQueue<HeavyJSON>& output_queue_;

};

}// namespace main_server

#endif //KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H
