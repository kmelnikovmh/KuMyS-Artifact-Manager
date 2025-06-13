#include "validate_request_module.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#define DEBUG_MODE_VALIDATE_MODULE_
#ifdef DEBUG_MODE_VALIDATE_MODULE_
    #define debug_cout_valmod std::cout
#else
    #define debug_cout_valmod if (false) std::cout
#endif

namespace kymus_proxy_server {

// Путь к файлу с заблокированными IP (должен быть доступен из конфигурации)
const std::string BLOCKED_IPS_FILE = "blocked-ip-list.txt";

Proxy::Proxy() : Fastcgipp::Request<wchar_t>(5 * 1024) {};

// Функция для чтения заблокированных IP из файла
std::vector<std::string> read_blocked_ips() {
    std::vector<std::string> blocked_ips;
    std::ifstream file(BLOCKED_IPS_FILE);
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Извлекаем IP из строки (первый элемент до пробела)
        size_t first_space = line.find(' ');
        if (first_space != std::string::npos) {
            blocked_ips.push_back(line.substr(0, first_space));
        }
    }
    
    return blocked_ips;
}

bool Proxy::validate_request() {
    bool validate = true;

    // 1. Получаем IP клиента из переменных окружения
    // extern char **environ;
    // if (environ[0]) std::cout << "YAHOO" << std::endl;
    // std::string client_ip;
    // if (environment().find(L"REMOTE_ADDR") != environment().end()) {
    //     std::wstring wip = environment().at(L"REMOTE_ADDR");
    //     client_ip = std::string(wip.begin(), wip.end());
    // } else {
    //     debug_cout_valmod << "Could not get client IP" << std::endl;
    //     return false;
    // }

    debug_cout_valmod << "Validating request from IP: " << client_ip << std::endl;

    // 2. Загружаем список заблокированных IP
    std::vector<std::string> blocked_ips = read_blocked_ips();
    
    // 3. Проверяем, есть ли IP клиента в списке заблокированных
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