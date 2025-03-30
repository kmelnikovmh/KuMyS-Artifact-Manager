//
// Created by Kymus-team on 2/22/25.
//

// TODO
#include "../include/HttpServer.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <cpprest/details/basic_types.h>
#include <utility>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace http;
using namespace utility;

main_server::HttpServer::HttpServer(const std::string&           url,
                                    folly::MPMCQueue<LightJSON>& input_queue,
                                    folly::MPMCQueue<HeavyJSON>& output_queue)
    : listener(url)
    , input_queue_(input_queue)
    , output_queue_(output_queue) {

    //   listener.support(methods::GET, [this](const http_request &httpRequest) {
    //   handle_get_request(httpRequest); });
    listener.support(methods::POST, [this](const http_request& httpRequest) { handle_post_request(httpRequest); });
}

void main_server::HttpServer::start() {
    try {
        listener.open().wait();
        std::cout << "HTTP Server started at: " << listener.uri().to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Server start failed: " << e.what() << std::endl;
    }
}

void main_server::HttpServer::stop() {
    listener.close().wait();
}

// TODO
void main_server::HttpServer::handle_get_request(const web::http::http_request& request) {
}

void main_server::HttpServer::handle_post_request(const web::http::http_request& request) {
    request.extract_json().then([this, request](const pplx::task<json::value>& task) {
        try {
            auto      json_body = task.get();
            LightJSON lightJson_request{.id           = json_body["id"].as_string(),
                                        .request_type = json_body["request_type"].as_string(),
                                        .name         = json_body["name"].as_string(),
                                        .version      = json_body["version"].as_string(),
                                        .architecture = json_body["architecture"].as_string(),
                                        .check_sum    = json_body["check_sum"].as_string()};

            if (validate_light_json(lightJson_request)) {
                input_queue_.blockingWrite(std::move(lightJson_request));
                request.reply(status_codes::Accepted, "Request queued");
            } else {
                request.reply(status_codes::BadRequest, "Invalid request body");
            }
        } catch (...) {
            request.reply(status_codes::BadRequest, "Invalid JSON");
        }
    });
}

void main_server::HttpServer::response_request() {
    HeavyJSON heavyJson;
    output_queue_.blockingRead(heavyJson);
    http_client client(U("http://127.0.0.1:7000"));
    
    json::value request_body;
    request_body[U("id")] = json::value::string(heavyJson.id);
    request_body[U("request_type")] = json::value::string(utility::conversions::to_string_t(heavyJson.request_type));
    request_body[U("name")] = json::value::string(utility::conversions::to_string_t(heavyJson.name));
    request_body[U("version")] = json::value::string(utility::conversions::to_string_t(heavyJson.version));
    request_body[U("architecture")] = json::value::string(utility::conversions::to_string_t(heavyJson.architecture));
    request_body[U("check_sum")] = json::value::string(utility::conversions::to_string_t(heavyJson.check_sum));
    request_body[U("file_size")] = json::value::number(heavyJson.file_size);
    request_body[U("created_at")] = json::value::string(utility::conversions::to_string_t(heavyJson.created_at));

    std::string base64_content = utility::conversions::to_base64(heavyJson.content);
    request_body[U("content")] = json::value::string(base64_content);

    json::value headers_json;
    for (const auto& [key, value] : heavyJson.headers) {
        headers_json[key] = json::value::string(value);
    }
    request_body[U("headers")] = headers_json;

    client.request(methods::POST, U("/"), request_body)
        .wait();
}

bool main_server::HttpServer::validate_light_json(const main_server::LightJSON& json) {
    const std::vector<std::reference_wrapper<const std::string>> fields = {
        json.id, json.request_type, json.name, json.version, json.architecture, json.check_sum};
    return std::all_of(fields.begin(), fields.end(), [](const std::string& fields) { return !fields.empty(); });
}
