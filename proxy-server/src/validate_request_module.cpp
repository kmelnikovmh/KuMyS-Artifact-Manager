#include <LightJson.hpp>                        // todo
#include <fastcgi++/request.hpp>
#include <iostream>

#define DEBUG_MODE_VALIDATE_MODULE_
#ifdef DEBUG_MODE_VALIDATE_MODULE_
    #define debug_cout_valmod std::cout
#else
    #define debug_cout_valmod if (false) std::cout
#endif

class Proxy : public Fastcgipp::Request<wchar_t> {
public:
    // post max size 5kilobit
    Proxy() : Fastcgipp::Request<wchar_t>(5 * 1024) {}

private:
    bool validate_request() {
        bool validate = true;
        // todo

        if (validate) {
            debug_cout_valmod << "Request is OK\n";
        } else {
            debug_cout_valmod << "Request is FUUUU\n";
        }
        return validate;
    }
    void response_pass_to_main_server() {
        out << "Status: 305 Use Proxy\r\n\r\n";
        out << "Proxy_pass\n";
        debug_cout_valmod << "Status: 305 Use Proxy\n";
    }
    void response_error_to_client() {
        out << "Status: 400 Bad Request\r\n\r\n";
        out << "Bad\n";
        debug_cout_valmod << "Status: 400 Bad Request\n";
    }

    bool response() {
        debug_cout_valmod << "-------------------------------- open request\n";
        if (validate_request()) {
            response_pass_to_main_server();
        } else {
            response_error_to_client();
        }
        debug_cout_valmod << "-------------------------------- close\n\n";
        return true;
    }
};

#include <fastcgi++/manager.hpp>

// validate_module_port (example: 5000")
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid input parameter\n";
        return 1;
    }
    Fastcgipp::Manager<Proxy> manager;
    manager.setupSignals();
    if (manager.listen("127.0.0.1", argv[1]) == false) {
        return 1;
    }
    manager.start();
    std::cout << "Validate module listen " << argv[1] << " port from nginx.\n";

    std::cout << "Validate module running. Press Enter to exit...\n\n";
    std::cin.get();
    manager.stop();
    manager.join();
}
