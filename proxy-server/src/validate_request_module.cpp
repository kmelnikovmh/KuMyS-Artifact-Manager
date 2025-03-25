#include <fastcgi++/request.hpp>
#include <LightJson.hpp>
#include <iostream>

class Proxy: public Fastcgipp::Request<wchar_t> {
public:
    Proxy():
        Fastcgipp::Request<wchar_t>(5*1024)         // post max size 5kilobit
    {}

private:
    bool validate_request(){
        bool validate = true;
        // todo
        std::cout << validate << "\n";
        return validate;
    }
    void response_pass_to_main_server(){
        out << "Status: 305 Use Proxy\r\n\r\n";
        out << "Proxy_pass\n";
        std::cout << "Status: 305 Use Proxy\n";
    }
    void response_error_to_client(){
        out << "Status: 400 Bad Request\r\n\r\n";
        out << "Bad\n";
        std::cout << "Status: 400 Bad Request\n";
    }

    bool response() {
        std::cout << "-------------------------------- open request\n";
        if (validate_request()) {
            response_pass_to_main_server();
        } else {
            response_error_to_client();
        }
        std::cout << "-------------------------------- close request\n";
        return true;
    }
};
 
#include <fastcgi++/manager.hpp>
 
// port (example: 5000")
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Invalid input parameter\n";
        return 1;
    }

    Fastcgipp::Manager<Proxy> manager;
    manager.setupSignals();
    manager.listen("127.0.0.1", argv[1]);
    manager.start();
    std::cout << "Validate_request_module start and listen: 127.0.0.1 " << argv[1] << "\n";
    manager.join();
 
    return 0;
}