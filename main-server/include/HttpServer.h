//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H
#define KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H

#include <cpprest/http_listener.h>

namespace main_server{
class HttpServer{
public:
    explicit HttpServer(std::string url);
    ~HttpServer();

    void start();
    void stop();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    HttpServer(HttpServer&&) = delete;
    HttpServer& operator=(HttpServer&&) = delete;
private:
    void request_routes();
    void handle_request(web::http::http_request request);

    web::http::experimental::listener::http_listener listener;

};

}// namespace main_server

#endif //KUMYS_ARTIFACT_MANAGER_HTTPMODULE_H
