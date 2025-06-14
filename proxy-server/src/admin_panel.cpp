#include "admin_panel.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>

// Добавить дебаг вывод
#define DEBUG_MODE_VALIDATE_MODULE_
#ifdef DEBUG_MODE_VALIDATE_MODULE_
    #define debug_cout_valmod std::cout
#else
    #define debug_cout_valmod if (false) std::cout
#endif

namespace kymus_proxy_server {

// AdminPanel
std::string AdminPanel::trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

std::string AdminPanel::get_current_datetime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&in_time_t, &tm_buf);
    
    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

AdminPanel::AdminPanel(const std::string& block_file_path) 
    : m_block_file_path(block_file_path) {
    
    std::ifstream test_file(block_file_path);
    if (!test_file.good()) {
        std::ofstream create_file(block_file_path);
        if (create_file) {
            create_file.close();
        }
    }
}

std::map<std::string, std::string> AdminPanel::block_user(
    const std::string& user_ip, 
    const std::string& reason) {
    std::lock_guard<std::mutex> lock(m_file_mutex);
    
    std::map<std::string, std::string> new_entry;
    new_entry["ip"] = user_ip;
    new_entry["block_date"] = get_current_datetime();
    new_entry["reason"] = reason;
    new_entry["status"] = "Активна";
    
    std::ofstream out_file(m_block_file_path, std::ios::app);
    if (out_file) {
        out_file << new_entry["ip"] << " " 
                 << new_entry["block_date"] << " " 
                 << new_entry["reason"] << "\n";
    }
    
    return new_entry;
}

void AdminPanel::unblock_user(const std::string& user_ip) {
    std::lock_guard<std::mutex> lock(m_file_mutex);
    
    std::ifstream in_file(m_block_file_path);
    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(in_file, line)) {
        if (line.find(user_ip + " ") == 0) {
            continue;
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    in_file.close();
    
    std::ofstream out_file(m_block_file_path, std::ios::trunc);
    for (const auto& l : lines) {
        out_file << l << "\n";
    }
}

std::vector<std::map<std::string, std::string>> AdminPanel::get_blocked_users() {
    std::lock_guard<std::mutex> lock(m_file_mutex);
    std::vector<std::map<std::string, std::string>> result;
    std::ifstream in_file(m_block_file_path);
    std::string line;
    
    while (std::getline(in_file, line)) {
        if (line.empty()) continue;
        
        size_t first_space = line.find(' ');
        if (first_space == std::string::npos) continue;
        
        size_t another_space = line.find(' ', first_space + 1);
        size_t second_space = line.find(' ', another_space + 1);
        if (second_space == std::string::npos) continue;
        
        std::map<std::string, std::string> entry;
        entry["ip"] = line.substr(0, first_space);
        entry["block_date"] = line.substr(first_space + 1, second_space - first_space - 1);
        entry["reason"] = line.substr(second_space + 1);
        entry["status"] = "Активна";
        
        result.push_back(entry);
    }
    return result;
}

// NginxListener
void NginxListener::handle_request(const web::http::http_request& request) {
    auto path = request.relative_uri().path();
    auto method = request.method();
    
    if (path == "/blacklist") {
        if (method == web::http::methods::GET) {
            auto blocked_users = m_admin_panel.get_blocked_users();
            web::json::value json_response = web::json::value::array();
            
            for (size_t i = 0; i < blocked_users.size(); i++) {
                web::json::value entry;
                entry["ip"] = web::json::value::string(blocked_users[i]["ip"]);
                entry["block_date"] = web::json::value::string(blocked_users[i]["block_date"]);
                entry["reason"] = web::json::value::string(blocked_users[i]["reason"]);
                entry["status"] = web::json::value::string(blocked_users[i]["status"]);
                
                json_response[i] = entry;
            }
            
            request.reply(web::http::status_codes::OK, json_response);
        }
        else if (method == web::http::methods::POST) {
            request.extract_json().then([=](web::json::value body) {
                try {
                    std::string ip = body["ip"].as_string();
                    std::string reason = body["reason"].as_string();
                    
                    auto new_entry = m_admin_panel.block_user(ip, reason);
                    
                    web::json::value json_response;
                    json_response["ip"] = web::json::value::string(new_entry["ip"]);
                    json_response["block_date"] = web::json::value::string(new_entry["block_date"]);
                    json_response["reason"] = web::json::value::string(new_entry["reason"]);
                    json_response["status"] = web::json::value::string(new_entry["status"]);
                    
                    request.reply(web::http::status_codes::Created, json_response);
                }
                catch (const std::exception& e) {
                    request.reply(web::http::status_codes::BadRequest, "Invalid request body");
                }
            });
        }
        else {
            request.reply(web::http::status_codes::MethodNotAllowed);
        }
    } else if (path.find("/blacklist/") == 0) {
        if (method == web::http::methods::DEL) {
            std::string ip = path.substr(strlen("/blacklist/"));
            ip = web::uri::decode(ip);
            
            m_admin_panel.unblock_user(ip);
            
            request.reply(web::http::status_codes::NoContent);
        }
        else {
            request.reply(web::http::status_codes::MethodNotAllowed);
        }
    }
    else if (path == "/admin" || path == "/admin/") {
        std::ifstream file("admin.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
            request.reply(web::http::status_codes::OK, content, "text/html");
        } else {
            request.reply(web::http::status_codes::NotFound, "Admin panel not found");
        }
    }
    else if (path == "/admin/blacklist" || path == "/admin/blacklist/") {
        std::ifstream file("blacklist.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
            request.reply(web::http::status_codes::OK, content, "text/html");
        } else {
            request.reply(web::http::status_codes::NotFound, "Blacklist page not found");
        }
    }
    else {
        request.reply(web::http::status_codes::NotFound);
    }
}

NginxListener::NginxListener(const std::string& nginx_uri, AdminPanel& admin_panel)
    : m_listener(nginx_uri), 
      m_admin_panel(admin_panel) {
    
    m_listener.support(std::bind(&NginxListener::handle_request, this, std::placeholders::_1));
}

void NginxListener::start() {
    m_listener.open().wait();
}

void NginxListener::close() {
    m_listener.close().wait();
}

} // namespace kymus_proxy_server