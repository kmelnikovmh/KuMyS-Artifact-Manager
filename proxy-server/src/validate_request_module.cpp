#include "validate_request_module.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h>

#define DEBUG_MODE_VALIDATE_MODULE_
#ifdef DEBUG_MODE_VALIDATE_MODULE_
    #define debug_cout_valmod std::cout
#else
    #define debug_cout_valmod if (false) std::cout
#endif

namespace kymus_proxy_server {

// BlockedList
std::string BlockedList::m_blocked_list_filepath = "";
void BlockedList::set_blocked_list_filepath(const std::string & string) {
    m_blocked_list_filepath = string;
}

std::vector<std::string> BlockedList::read_blocked_ips() {
    std::vector<std::string> blocked_ips;
    std::ifstream file(m_blocked_list_filepath);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t first_space = line.find(' ');
        if (first_space != std::string::npos) {
            blocked_ips.push_back(line.substr(0, first_space));
        }
    }
    
    return blocked_ips;
}

std::string BlockedList::address_class_to_string(const Fastcgipp::Address& addr) {
    char str[INET6_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET6, addr.m_data.data(), str, INET6_ADDRSTRLEN)) {
        std::string ipStr(str);

        // Удаляем IPv4-mapped префикс ::ffff:
        const std::string prefix = "::ffff:";
        if (ipStr.rfind(prefix, 0) == 0) {
            return ipStr.substr(prefix.size());
        }
        return ipStr;
    }
    return "Invalid IP";
}

// Proxy
Proxy::Proxy() : Fastcgipp::Request<wchar_t>(5 * 1024) {};

bool Proxy::validate_request() {
    bool validate = true;

    std::string client_ip = BlockedList::address_class_to_string(environment().remoteAddress);
    debug_cout_valmod << "Validating request from IP: " << client_ip << std::endl;

    std::vector<std::string> blocked_ips = BlockedList::read_blocked_ips();
    if (std::find(blocked_ips.begin(), blocked_ips.end(), client_ip) != blocked_ips.end()) {
        validate = false;
        debug_cout_valmod << "Client IP " << client_ip << " is blocked" << std::endl;
    }

    if (validate) {
        debug_cout_valmod << "Request is OK" << std::endl;
    } else {
        debug_cout_valmod << "Request is blocked" << std::endl;
    }
    return validate;
}

void Proxy::set_pass_to_main_server() {
    out << "Status: 305 Use Proxy\r\n\r\n";
    out << "Proxy_pass\n";
    debug_cout_valmod << "Status: 305 Use Proxy" << std::endl;
}

void Proxy::set_error_to_client() {
    out << "Status: 403 Forbidden\r\n\r\n";
    out << "Your IP address has been blocked\n";
    debug_cout_valmod << "Status: 403 Forbidden" << std::endl;
}

bool Proxy::response() {
    debug_cout_valmod << "-------------------------------- open request" << std::endl;
    if (validate_request()) {
        set_pass_to_main_server();
    } else {
        set_error_to_client();
    }
    debug_cout_valmod << "-------------------------------- close" << std::endl << std::endl;
    return true;
}
}