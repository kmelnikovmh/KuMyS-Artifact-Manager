#include "validate_request_module.h"

// validate_module_port (example: 5000")

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid input parameter\n";
        return 1;
    }
    
    Fastcgipp::Manager<kymus_proxy_server::Proxy> manager;
    manager.setupSignals();
    if (manager.listen("0.0.0.0", argv[1]) == false) {
        return 1;
    }
    manager.start();
    std::cout << "Validate module listen " << argv[1] << " port from nginx.\n";

    std::cout << "Validate module running. Press Enter to exit...\n\n";
    std::cin.get();
    manager.stop();
    manager.join();
}
