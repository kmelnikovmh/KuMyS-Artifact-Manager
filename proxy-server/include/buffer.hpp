#ifndef PROXY_BUFFER_
#define PROXY_BUFFER_

#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>

// #include "LightJson.h"            // todo
// #include "HeavyJson.h"            // todo
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>

namespace kymus_proxy_server {
class PromiseAtomicMap {
private:
    mutable std::mutex m_map_mtx;
    std::unordered_map<uint64_t, std::promise<std::pair<std::vector<uint8_t>, web::json::object>>> m_map;

public:
    std::future<std::pair<std::vector<uint8_t>, web::json::object>> create_and_return_future(uint64_t original_id_client);

    void set_future(uint64_t original_id_client, std::vector<uint8_t> content, web::json::object json_headers);
    void erase_promise(uint64_t original_id_client);

    static uint64_t simple_hash(const std::string& client_ip);
};

class JsonSender {
private:
    web::http::client::http_client m_sender;

public:
    JsonSender(const std::string &uri);

    void parsing_uri_and_set_json(web::http::uri uri, web::json::value &json_object);
    web::http::status_code json_send(const web::http::http_request& request, uint64_t original_id_client);
};

class NginxListener {
private:
    PromiseAtomicMap& m_map;
    web::http::experimental::listener::http_listener listener;
    std::string main_server_uri;

    static JsonSender& get_json_sender(const std::string &uri);

    // HEAD FUNCTION
    void handle_request(const web::http::http_request& request);

public:
    NginxListener(const std::string& nginx_uri, const std::string& main_server_uri, PromiseAtomicMap& map);
    void start();
    void close();
};

class MainListener {
private:
    PromiseAtomicMap& m_map;
    web::http::experimental::listener::http_listener listener;

    // HEAD FUNCTION
    void handle_response(const web::http::http_request& request);

public:
    MainListener(const std::string& main_uri, PromiseAtomicMap& map);
    void start();
    void close();
};
}
#endif
