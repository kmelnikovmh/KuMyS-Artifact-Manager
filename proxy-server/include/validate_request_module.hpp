#ifndef PROXY_VALIDATE_MODULE_
#define PROXY_VALIDATE_MODULE_

#include <iostream>
#include <string>
#include <vector>

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

namespace kymus_proxy_server {

class BlockedList {
private:
    static std::string m_blocked_list_filepath;

    static std::vector<std::string> read_blocked_ips();
    static std::string address_class_to_string(const Fastcgipp::Address& addr);

public:
    static void set_blocked_list_filepath(const std::string & string);

    friend class Proxy;
};

class Proxy : public Fastcgipp::Request<wchar_t> {
private:
    bool validate_request();

    void set_pass_to_main_server();
    void set_error_to_client();

    bool response();

public:
    Proxy();
};

} // namespace kymus_proxy_server

#endif