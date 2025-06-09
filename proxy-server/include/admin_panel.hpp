#ifndef PROXY_ADMIN_PANEL_
#define PROXY_ADMIN_PANEL_

#include <iostream>
#include <string>
#include <vector>

// #include "LightJson.h"            // todo
// #include "HeavyJson.h"            // todo
#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>

namespace kymus_proxy_server {

class NginxListener {
private:
    web::http::experimental::listener::http_listener m_listener;

    // HEAD FUNCTION
    void handle_request(const web::http::http_request& request);

public:
    NginxListener(const std::string& nginx_uri);
    void start();
    void close();
};

class BlockedUsers {
private:
    std::string m_filepath_blocked_users_list;

    friend class NginxListener;

    void block_user(const std::string& user_ip);
    void unblock_user(const std::string& user_ip);
    std::vector<std::string> get_blocked_users();

public:
    BlockedUsers(const std::string& filepath_blocked_users_list);
}
}
#endif