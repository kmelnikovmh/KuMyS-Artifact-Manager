//
// Created by Kymus-team on 2/22/25.
//

// TODO
#include "../include/HttpServer.h"
#include <algorithm>
#include <vector>
#include <functional>

using namespace web;
using namespace http;
using namespace utility;

main_server::HttpServer::HttpServer(std::string url, folly::MPMCQueue<LightJSON> &input_queue,
                                    folly::MPMCQueue<HeavyJSON> &output_queue) : listener(url),
                                                                                 input_queue_(input_queue),
                                                                                 output_queue_(output_queue) {

    //listener.support(methods::GET, [this](const http_request &httpRequest) { handle_get_request(httpRequest); });
    listener.support(methods::POST, [this](const http_request &httpRequest) { handle_post_request(httpRequest); });


}

void main_server::HttpServer::start() {
    try{
        listener.open().wait();
        std::cout << "HTTP Server started at: " << listener.uri().to_string() << std::endl;
    } catch(const std::exception& e) {
        std::cerr << "Server start failed: " << e.what() << std::endl;
    }

}

void main_server::HttpServer::stop() {
    listener.close().wait();
}

//TODO
void main_server::HttpServer::handle_post_request(web::http::http_request request) {
}

void main_server::HttpServer::handle_get_request(web::http::http_request request) {
    request.extract_json().then([this, request](pplx::task<json::value> task){
        try{
            auto json_body = task.get();
            LightJSON lightJson_request {
                    .id = json_body["id"].as_string(),
                    .request_type = json_body["request_type"].as_string(),
                    .name = json_body["name"].as_string(),
                    .version = json_body["version"].as_string(),
                    .architecture = json_body["architecture"].as_string(),
                    .check_sum = json_body["check_sum"].as_string()
            };

            if(validate_light_json(lightJson_request)){
                input_queue_.blockingWrite(std::move(lightJson_request));
                request.reply(status_codes::Accepted, "Request queued");
            }
            else{
                request.reply(status_codes::BadRequest, "Invalid request body");
            }
        }
        catch (...){
            request.reply(status_codes::BadRequest, "Invalid JSON");
        }
    });
}

bool main_server::HttpServer::validate_light_json(const main_server::LightJSON &json) {
    const std::vector<std::reference_wrapper<const std::string>> fields = {
            json.id,
            json.request_type,
            json.name,
            json.version,
            json.architecture,
            json.check_sum
    };
    return std::all_of(fields.begin(), fields.end(), [](const std::string& fields){
        return !fields.empty();
    });

}

