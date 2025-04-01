#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include "LightJson.hpp"            // todo
#include "HeavyJson.hpp"            // todo
#include "buffer.h"
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>

#define DEBUG_MODE_BUFFER_
#ifdef DEBUG_MODE_BUFFER_
    #define debug_cout_buffer_ std::cout
#else
    #define debug_cout_buffer_ if (false) std::cout
#endif

namespace kymus_proxy_server {
class PromiseAtomicMap {
private:
    mutable std::mutex m_map_mtx;
    std::unordered_map<uint64_t, std::promise<std::pair<std::vector<uint8_t>, web::json::object>>> m_map;

public:
    std::future<std::pair<std::vector<uint8_t>, web::json::object>> create_and_return_future(uint64_t original_id_client) {
        std::unique_lock l(m_map_mtx);
        std::promise<std::pair<std::vector<uint8_t>, web::json::object>> req_promise;
        std::future<std::pair<std::vector<uint8_t>, web::json::object>> req_future = req_promise.get_future();
        m_map.emplace(original_id_client, std::move(req_promise));
        return std::move(req_future);
    }

    void set_future(uint64_t original_id_client, std::vector<uint8_t> content, web::json::object json_headers) {
        std::unique_lock l(m_map_mtx);
        auto it_m_map = m_map.find(original_id_client);
        it_m_map->second.set_value({content, json_headers});
    }

    void erase_promise(uint64_t original_id_client) {
        std::unique_lock l(m_map_mtx);
        auto it_m_map = m_map.find(original_id_client);
        m_map.erase(it_m_map);
    }

    static uint64_t simple_hash(const std::string& client_ip) {
        auto now = std::chrono::system_clock::now();
        auto time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count();
        
        std::hash<std::string> hasher;
        return hasher(std::to_string(time_ns) + client_ip);
    }
};

class JsonSender {
private:
    web::http::client::http_client m_sender;

public:
    JsonSender(const std::string &uri) : m_sender(uri) {}
    
    void parsing_uri_and_set_json(web::http::uri uri, web::json::value &json_object) {
        std::string uri_str = uri.to_string();
        debug_cout_buffer_ << uri_str << "\n";

        size_t first_slash = uri_str.find('/');
        size_t second_slash = uri_str.find('/', first_slash + 1);
        if (first_slash != std::string::npos && second_slash != std::string::npos) {
            std::string repo = uri_str.substr(first_slash + 1, second_slash - first_slash - 1);
            debug_cout_buffer_ << repo << "\n";
            json_object[U("repo")] = web::json::value::string(repo);
        } else {
            // Обработать неверный путь
        }

        size_t last_slash = uri_str.rfind('/');
        if (last_slash != std::string::npos && second_slash < last_slash) {
            std::string path = uri_str.substr(second_slash + 1, last_slash - second_slash - 1);
            debug_cout_buffer_ << path << "\n";
            json_object[U("path")] = web::json::value::string("path/to/package");
        } else {
            // Обработать неверный путь
        }

        size_t dot_pos = uri_str.rfind('.');
        if (dot_pos == std::string::npos) {
            json_object[U("request_type")] = web::json::value::string("update");
            std::string name = uri_str.substr(last_slash + 1);
            debug_cout_buffer_ << name << "\n";
            json_object[U("name")] = web::json::value::string(name);
            json_object[U("version")] = web::json::value::string("-");
            json_object[U("architecture")] = web::json::value::string("-");
        } else {
            json_object[U("request_type")] = web::json::value::string("install");

            size_t first_underscore = uri_str.find('_', last_slash + 1);
            if (first_underscore != std::string::npos) {
                std::string name = uri_str.substr(last_slash + 1, first_underscore - last_slash - 1);
                debug_cout_buffer_ << name << "\n";
                json_object[U("name")] = web::json::value::string(name);
            } else {
                // Обработать неверный путь
            }

            size_t last_underscore = uri_str.rfind('_');
            if (last_underscore != std::string::npos && first_underscore < last_underscore) {
                std::string version = uri_str.substr(first_underscore + 1, last_underscore - first_underscore - 1);
                debug_cout_buffer_ << version << "\n";
                json_object[U("version")] = web::json::value::string(version);
            } else {
                // Обработать неверный путь
            }

            if (last_underscore < dot_pos) {
                std::string architecture = uri_str.substr(last_underscore + 1, dot_pos - last_underscore - 1);
                debug_cout_buffer_ << architecture << "\n";
                json_object[U("architecture")] = web::json::value::string(architecture);
            } else {
                // Обработать неверный путь
            }
        }
    }

    web::http::status_code json_send(const web::http::http_request& request, uint64_t original_id_client) {
        web::json::value json_object = web::json::value::object();
        json_object[U("id")] = web::json::value::string(std::to_string(original_id_client));        // Сделать чиселкой
        json_object[U("check_sum")] = web::json::value::string("sha256:todo");

        parsing_uri_and_set_json(request.relative_uri(), json_object);

        web::http::http_response response = m_sender.request(web::http::methods::POST, U(""), json_object).get();
        return response.status_code();
    }
};

class NginxListener {
private:
    PromiseAtomicMap& m_map;
    web::http::experimental::listener::http_listener listener;
    std::string main_server_uri;

    // HEAD FUNCTION
    void handle_request(const web::http::http_request& request) {
        // Accept
        debug_cout_buffer_ << "-------------------------------- open request\n";
        debug_cout_buffer_ << request.to_string();

        // Hashing
        uint64_t original_id_client = PromiseAtomicMap::simple_hash(request.headers().find("X-Real-IP")->second);
        std::cout << original_id_client << "\n\n";
        std::future<std::pair<std::vector<uint8_t>, web::json::object>> req_future = m_map.create_and_return_future(original_id_client);

        // Send request
        JsonSender &m_json_sender = get_json_sender(main_server_uri);
        web::http::status_code main_server_code = m_json_sender.json_send(request, original_id_client);
        if (main_server_code == 400) {
            request.reply(web::http::status_codes::InternalError, "Invalid request from proxy-server to main-server\n");
            debug_cout_buffer_ << "Invalid request from proxy-server to main-server\n";
            debug_cout_buffer_ << "-------------------------------- close request\n\n";
            return;
        }
        
        // Waiting and send response
        std::thread([&, req_future = std::move(req_future)]() mutable {
            debug_cout_buffer_ << ". waiting promise\n\n\n";
            auto [content, json_headers] = req_future.get();
            
            m_map.erase_promise(original_id_client);

            web::http::http_response response(200);
            
            // Заголовки
            for (auto it : json_headers) {
                response.headers().add(it.first, it.second.as_string());
            }
            // Бинарь, тело
            response.set_body(content);

            request.reply(response);
            debug_cout_buffer_ << "-------------------------------- close\n\n";
        }).detach();
    }

    static JsonSender& get_json_sender(const std::string &uri) {
        static JsonSender m_sender(uri);
        return m_sender;
    } 
public:
    NginxListener(const std::string& nginx_uri, const std::string& main_server_uri, PromiseAtomicMap& map) : 
        listener(nginx_uri), 
        main_server_uri(main_server_uri), 
        m_map(map) {
        listener.support(std::bind(&NginxListener::handle_request, this, std::placeholders::_1));
    }

    void start() {
        listener.open().wait();
        std::cout << "NginxListener start\n";
    }

    void close(){
        listener.close().wait();
        std::cout << "NginxListener close\n";
    }
};


class MainListener {
private:
    PromiseAtomicMap& m_map;
    web::http::experimental::listener::http_listener listener;

    // HEAD FUNCTION
    void handle_response(const web::http::http_request& request) {
        debug_cout_buffer_ << request.to_string();          // Фигня из-за body, работает так себе

        web::json::value json = request.extract_json().get();

        utility::string_t id_client_t = request.headers().find("id")->second;
        std::string id_client_std = utility::conversions::to_utf8string(id_client_t);
        uint64_t id_client_uint64 = std::stoull(id_client_std);

        web::json::object json_headers = json["headers"].as_object();

        std::string content_str = json["content"].as_string();
        std::vector<uint8_t> content = utility::conversions::from_base64(content_str);

        m_map.set_future(id_client_uint64, content, json_headers);
        debug_cout_buffer_ << ". set future\n";
    }
public:
    MainListener(const std::string& main_uri, PromiseAtomicMap& map) : 
        listener(main_uri), 
        m_map(map) {
        listener.support(std::bind(&MainListener::handle_response, this, std::placeholders::_1));
    }

    void start(){
        listener.open().wait();
        std::cout << "MainListener start\n";
    }
    void close(){
        listener.close().wait();
        std::cout << "MainListener close\n";
    }
};
}

using namespace kymus_proxy_server;

// variables: buffer_listener_nginx_port, main_server_ip, main_server_port, buffer_listener_main_port, buffer_ip_out 
// example: 6000 127.0.0.1 6500 7000 optional: 127.0.0.1
int main (int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Invalid input parameters\n";
        return 1;
    }
    std::string buffer_listener_nginx_port = argv[1];
    std::string main_server_ip = argv[2];
    std::string main_server_port = argv[3];
    std::string buffer_listener_main_port = argv[4];

    std::string buffer_ip;
    if (argc==6) {
        buffer_ip = argv[5];
    } else {
        buffer_ip = "unknow";
    }

    PromiseAtomicMap promise_map;
    NginxListener nginx("http://0.0.0.0:"+buffer_listener_nginx_port, "http://"+main_server_ip+":"+main_server_port, promise_map);
    MainListener main("http://0.0.0.0:"+buffer_listener_main_port, promise_map);
    
    nginx.start();
    main.start();
    std::cout << "\nBuffer listening to "<< buffer_listener_nginx_port <<" port from nginx.\n";
    std::cout << "Buffer connects to main server via "<<main_server_ip<<":"<<main_server_port<<" and wait response to " << buffer_ip <<":"<< buffer_listener_main_port << " port.\n";

    std::cout << "Buffer running. Press Enter to exit...\n\n";
    std::cin.get();

    nginx.close();
    main.close();
}