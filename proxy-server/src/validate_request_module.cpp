#include "validate_request_module.hpp"

#define DEBUG_MODE_VALIDATE_MODULE_
#ifdef DEBUG_MODE_VALIDATE_MODULE_
    #define debug_cout_valmod std::cout
#else
    #define debug_cout_valmod if (false) std::cout
#endif

namespace kymus_proxy_server {
// post max size 5kilobit
Proxy::Proxy() : Fastcgipp::Request<wchar_t>(5 * 1024) {};

bool Proxy::validate_request() {
    bool validate = true;

    // todo - Check ip users from file

    if (validate) {
        debug_cout_valmod << "Request is OK" << std::endl;
    } else {
        debug_cout_valmod << "Request is FUUUU" << std::endl;
    }
    return validate;
}

void Proxy::set_pass_to_main_server() {
    out << "Status: 305 Use Proxy\r\n\r\n";
    out << "Proxy_pass\n";
    debug_cout_valmod << "Status: 305 Use Proxy" << std::endl;
}

void Proxy::set_error_to_client() {
    out << "Status: 400 Bad Request\r\n\r\n";
    out << "Bad\n";
    debug_cout_valmod << "Status: 400 Bad Request" << std::endl;
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
