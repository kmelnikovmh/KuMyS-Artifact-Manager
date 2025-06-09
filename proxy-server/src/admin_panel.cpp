#include <admin_panel.hpp>

#define DEBUG_MODE_BUFFER_
#ifdef DEBUG_MODE_BUFFER_
    #define debug_cout_buffer_ std::cout
#else
    #define debug_cout_buffer_ if (false) std::cout
#endif

namespace kymus_proxy_server {

// NginxListere
void NginxListener::handle_request(const web::http::http_request& request) {
    // TODO 
}

NginxListener::NginxListener(const std::string& nginx_uri) : m_listener(nginx_uri) {
        m_listener.support(std::bind(&NginxListener::handle_request, this, std::placeholders::_1));
    }

void NginxListener::start() {
    m_listener.open().wait();
    std::cout << "NginxListener start" << std::endl;
}

void NginxListener::close(){
    m_listener.close().wait();
    std::cout << "NginxListener close" << std::endl;
}

//BlockedUsers
BlockedUsers::BlockedUsers() {
    // TODO
}

}