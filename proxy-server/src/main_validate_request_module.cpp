#include "validate_request_module.hpp"

// validate_module_port (example: 63350")

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Invalid input parameter\n";
        return 1;
    }

    std::string blocked_list_filepath = argv[2];
    kymus_proxy_server::BlockedList::set_blocked_list_filepath(blocked_list_filepath);
    
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
