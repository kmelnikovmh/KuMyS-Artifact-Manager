#include <admin_panel.hpp>

// variables: admin_panel_listener_nginx_port
// example: 63340

int main (int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Invalid input parameters\n";
        return 1;
    }
    std::string admin_panel_listener_nginx_port = argv[1];

    kymus_proxy_server::NginxListener nginx("http://0.0.0.0:"+buffer_listener_nginx_port);
    
    nginx.start();
    std::cout << "\nAdmin-Panel listening to "<< admin_panel_listener_nginx_port <<" port from nginx.\n";

    std::cout << "Admin-Panel running. Press Enter to exit...\n\n";
    std::cin.get();

    nginx.close();
}