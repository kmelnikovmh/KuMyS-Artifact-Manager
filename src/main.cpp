//
// Created by Kymus-team on 2/22/25.
//
#include "../include/DatabaseManager.h"
#include "../include/HttpServer.h"
#include <iostream>
#include <string>

int main() {
    main_server::DatabaseManger::init("mongodb://127.0.0.1:<PORT>");

    main_server::HttpServer server("127.0.0.1:<PORT>");
    server.start();

    std::cout << "Server running. Press Enter to exit..." << std::endl;
    // TODO
    server.stop();

    return 0;
};
