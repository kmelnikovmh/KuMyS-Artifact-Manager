//
// Created by Kymus-team on 2/22/25.
//
#include "../include/HttpServer.h"
#include <algorithm>
#include <cpprest/details/basic_types.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <functional>
#include <utility>
#include <vector>

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
    , output_queue_(output_queue)
    , executor_(std::make_shared<folly::CPUThreadPoolExecutor>(std::thread::hardware_concurrency())) {

    listener.support(methods::POST, [this](const http_request& httpRequest) { handle_post_request(httpRequest); });
}

void main_server::HttpServer::start() {
    try {
        listener.open().wait();
        is_running_.store(true);
        std::thread([this] { process_loop(); }).detach();
        std::cout << "HTTP Server started at: " << listener.uri().to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Server start failed: " << e.what() << std::endl;
    }
}

void main_server::HttpServer::stop() {
    listener.close().wait();
    is_running_.store(false);
    executor_->join();
}

void main_server::HttpServer::process_loop() {
    while (is_running_.load()) {
        HeavyJSON package;
        std::cout << "TRYREAD HTTPSERVER" << std::endl;
        output_queue_.blockingRead(package);
        std::cout << "ACCLY READ HTTPSERVER" << std::endl;
        folly::coro::co_invoke([this, pkg = std::move(package)] { std::cout << "wth mane" << std::endl; return response_request(std::move(pkg)); })
            .scheduleOn(executor_.get())
            .start();
    }
}

// TODO
void main_server::HttpServer::handle_get_request(const web::http::http_request& request) {
}

void print(main_server::LightJSON json) {
    std::cout << "\tTO INSTALL\nID: " << json.id << "\n Request Type: " << json.request_type << "\n Name: " << json.name
                  << "\n Version: " << json.version << "\n Architecture: " << json.architecture << "\n Check Sum: "
                  << json.check_sum << "\n Repo: " << json.repo << "\n Path: " << json.path << std::endl;
}

void main_server::HttpServer::handle_post_request(const web::http::http_request& request) {
    std::cout << "GET" << std::endl;
    request.extract_json().then([this, request](const pplx::task<json::value>& task) {
        try {
            auto      json_body = task.get();
            LightJSON lightJson_request{.id           = json_body["id"].as_number().to_uint64(),
                                        .request_type = json_body["request_type"].as_string(),
                                        .name         = json_body["name"].as_string(),
                                        .version      = json_body["version"].as_string(),
                                        .architecture = json_body["architecture"].as_string(),
                                        .check_sum    = json_body["check_sum"].as_string(),
                                        .repo         = json_body["repo"].as_string(),
                                        .path         = json_body["path"].as_string()};
            print(lightJson_request);
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

folly::coro::Task<void> main_server::HttpServer::response_request(const main_server::HeavyJSON& heavyJson) {
    std::cout << "START DOING IT HELL" << std::endl;
    http_client client(U("http://proxy:63370"));

    json::value request_body;
    request_body[U("id")]           = json::value::number(heavyJson.id);
    request_body[U("request_type")] = json::value::string(utility::conversions::to_string_t(heavyJson.request_type));
    request_body[U("name")]         = json::value::string(utility::conversions::to_string_t(heavyJson.name));
    request_body[U("version")]      = json::value::string(utility::conversions::to_string_t(heavyJson.version));
    request_body[U("architecture")] = json::value::string(utility::conversions::to_string_t(heavyJson.architecture));
    request_body[U("check_sum")]    = json::value::string(utility::conversions::to_string_t(heavyJson.check_sum));
    request_body[U("repo")]         = json::value::string(utility::conversions::to_string_t(heavyJson.repo));
    request_body[U("path")]         = json::value::string(utility::conversions::to_string_t(heavyJson.path));
    request_body[U("file_size")]    = json::value::number(heavyJson.file_size);
    request_body[U("created_at")]   = json::value::string(utility::conversions::to_string_t(heavyJson.created_at));

    std::string base64_content = utility::conversions::to_base64(heavyJson.content);
    request_body[U("content")] = json::value::string(base64_content);

    json::value headers_json;
    for (const auto& [key, value] : heavyJson.headers) {
        headers_json[key] = json::value::string(value);
    }
    request_body[U("headers")] = headers_json;

    std::cout << "SEND\n" << std::endl;
    auto resp = client.request(methods::POST, U("/"), request_body).get();
    std::cout << resp.status_code() << std::endl;
    co_return;
}

bool main_server::HttpServer::validate_light_json(const main_server::LightJSON& json) {
    const std::vector<std::reference_wrapper<const std::string>> fields = {
        json.request_type, json.name, json.version, json.architecture, json.check_sum, json.repo, json.path};
    return std::all_of(fields.begin(), fields.end(), [](const std::string& fields) { return !fields.empty(); });
}
