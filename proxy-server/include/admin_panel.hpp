#ifndef PROXY_ADMIN_PANEL_
#define PROXY_ADMIN_PANEL_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>

#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>

namespace kymus_proxy_server {

class AdminPanel {
private:
    std::string m_block_file_path;
    std::mutex m_file_mutex;

    std::string get_current_datetime();
    std::string trim(const std::string &s);

public:
    AdminPanel(const std::string& block_file_path);
    
    std::map<std::string, std::string> block_user(const std::string& user_ip, const std::string& reason);
    
    void unblock_user(const std::string& user_ip);
    
    std::vector<std::map<std::string, std::string>> get_blocked_users();
};

class NginxListener {
private:
    web::http::experimental::listener::http_listener m_listener;
    AdminPanel& m_admin_panel;

    void handle_request(const web::http::http_request& request);

public:
    NginxListener(const std::string& nginx_uri, AdminPanel& admin_panel);
    void start();
    void close();
};

} // namespace kymus_proxy_server

#endif