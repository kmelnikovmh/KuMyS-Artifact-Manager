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
    std::unordered_map<std::string, std::promise<std::vector<uint8_t>>> m_map;

public:
    std::future<std::vector<uint8_t>> create_and_return_future(std::string original_id_client) {
        std::unique_lock l(m_map_mtx);
        std::promise<std::vector<uint8_t>> req_promise;
        std::future<std::vector<uint8_t>> req_future = req_promise.get_future();
        m_map.emplace(original_id_client, std::move(req_promise));
        l.unlock();
        return std::move(req_future);
    }

    void set_future(const std::string &original_id_client, std::vector<uint8_t> content) {
        std::unique_lock l(m_map_mtx);
        auto it_m_map = m_map.find(original_id_client);
        it_m_map->second.set_value(content);
    }

    void erase_promise(const std::string &original_id_client) {
        std::unique_lock l(m_map_mtx);
        auto it_m_map = m_map.find(original_id_client);
        m_map.erase(it_m_map);
    }
};

class JsonSender {
private:
    web::http::client::http_client m_sender;

public:
    JsonSender(const std::string &uri) : m_sender(uri) {}

    // Настроить поля, данные
    web::http::status_code json_send(const web::http::http_request& request) {
        web::json::value json_object = web::json::value::object();
        json_object[U("id")] = web::json::value::string(U(request.headers().find("X-Real-IP")->second));
        json_object[U("request_type")] = web::json::value::string(U("install"));
        json_object[U("name")] = web::json::value::string(U(request.request_uri().to_string()));
        json_object[U("version")] = web::json::value::string(U("-"));
        json_object[U("architecture")] = web::json::value::string(U("-"));
        json_object[U("check_sum")] = web::json::value::string(U("-"));

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

        // Send request
        JsonSender &m_json_sender = get_json_sender(main_server_uri);
        web::http::status_code main_server_code = m_json_sender.json_send(request);
        if (main_server_code == 400) {
            request.reply(web::http::status_codes::InternalError, "Invalid request from proxy-server to main-server\n");
            debug_cout_buffer_ << "Invalid request from proxy-server to main-server\n";
            debug_cout_buffer_ << "-------------------------------- close request\n\n";
            return;
        }
        
        // Waiting and send response
        std::thread([&]() {
            // Надо более сложную хэш функцию. Не поддерживает с одного ip несколько висячих запросов
            std::string original_id_client = request.headers().find("X-Real-IP")->second;

            std::future<std::vector<uint8_t>> req_future = m_map.create_and_return_future(original_id_client);
            debug_cout_buffer_ << ". waiting promise\n\n\n";
            std::vector<uint8_t> content = req_future.get();
            
            // Настроить заголовки
            m_map.erase_promise(original_id_client);
            web::http::http_response response(200);
            response.headers().set_content_type("application/x-debian-package");
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
        debug_cout_buffer_ << request.to_string();
        web::json::value json = request.extract_json().get();
        const std::string original_id_client = json["id"].as_string();
        // Разобраться с кодировкой бинарей из json
        // json["content"].as_string()  -->  std::vector<uint8_t> content
        std::vector<uint8_t> content = {0, 1, 2, 3, 4, 5};

        m_map.set_future(original_id_client, content);
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
    NginxListener nginx("http://127.0.0.1:"+buffer_listener_nginx_port, "http://"+main_server_ip+":"+main_server_port, promise_map);
    MainListener main("http://127.0.0.1:"+buffer_listener_main_port, promise_map);
    
    nginx.start();
    main.start();
    std::cout << "\nBuffer listening to "<< buffer_listener_nginx_port <<" port from nginx.\n";
    std::cout << "Buffer connects to main server via "<<main_server_ip<<":"<<main_server_port<<" and wait response to " << buffer_ip <<":"<< buffer_listener_main_port << " port.\n";

    std::cout << "Buffer running. Press Enter to exit...\n\n";
    std::cin.get();

    nginx.close();
    main.close();
}