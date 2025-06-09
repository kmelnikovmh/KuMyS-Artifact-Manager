#include "buffer.hpp"

// variables: buffer_listener_nginx_port, main_server_ip, main_server_port, buffer_listener_main_port, buffer_ip_out 
// example: 63360 127.0.0.1 6500 63370 optional: 127.0.0.1

int main (int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Invalid input parameters\n";
        return 1;
    }
    std::string buffer_listener_nginx_port = argv[1];
    std::string main_server_ip = argv[2];
    std::string main_server_port = argv[3];
    std::string buffer_listener_main_port = argv[4];

    std::string buffer_ip;
    if (argc==6) {
        buffer_ip = argv[5];
    } else {
        buffer_ip = "unknow";
    }

    kymus_proxy_server::PromiseAtomicMap promise_map;
    kymus_proxy_server::NginxListener nginx("http://0.0.0.0:"+buffer_listener_nginx_port, "http://"+main_server_ip+":"+main_server_port, promise_map);
    kymus_proxy_server::MainListener main("http://0.0.0.0:"+buffer_listener_main_port, promise_map);
    
    nginx.start();
    main.start();
    std::cout << "\nBuffer listening to "<< buffer_listener_nginx_port <<" port from nginx.\n";
    std::cout << "Buffer connects to main server via "<<main_server_ip<<":"<<main_server_port<<" and wait response to " << buffer_ip <<":"<< buffer_listener_main_port << " port.\n";

    std::cout << "Buffer running. Press Enter to exit...\n\n";
    std::cin.get();

    nginx.close();
    main.close();
}