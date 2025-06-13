#include "admin_panel.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <port> <blocked_ips_file>\n";
        return 1;
    }
    
    std::string port = argv[1];
    std::string blocked_ips_file = argv[2];
    
    std::string uri = "http://0.0.0.0:" + port;
    
    // Создаем AdminPanel
    kymus_proxy_server::AdminPanel admin_panel(blocked_ips_file);
    
    // Создаем и запускаем слушатель
    kymus_proxy_server::NginxListener nginx(uri, admin_panel);
    
    try {
        nginx.start();
        std::cout << "Admin-Panel listening on port " << port << "\n";
        std::cout << "Blocked IPs stored in: " << blocked_ips_file << "\n";
        std::cout << "Admin panel available at: http://localhost:" << port << "/admin\n";
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
        nginx.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}