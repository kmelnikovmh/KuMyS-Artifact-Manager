//
// Created by Kymus-team on 2/22/25.
//

#include "ServerApp.h"
#include "../include/DatabaseManager.h"
#include "HeavyJson.h"
#include "../include/HttpServer.h"
#include "LightJson.h"
#include "../include/PackageDownloader.h"
#include "../include/RequestHandler.h"
#include <folly/MPMCQueue.h>
#include <iostream>
#include <string>

constexpr std::size_t QUEUE_CAPACITY   = 1000;
constexpr std::size_t DB_PORT          = 27017;
constexpr std::size_t HTTP_SERVER_PORT = 38081;

int runServerApp(){
    try {
        std::cout << "Start server-setup" << std::endl;

        // DB-init
        main_server::DatabaseManager databaseManager("mongodb://root:123@mongodb:27017");
        databaseManager.clean();
        std::cout << "Database is init!" << std::endl;

        // Queue-s init
        folly::MPMCQueue<main_server::LightJSON> input_queue(QUEUE_CAPACITY);
        folly::MPMCQueue<main_server::LightJSON> download_queue(QUEUE_CAPACITY);
        folly::MPMCQueue<main_server::HeavyJSON> output_queue(QUEUE_CAPACITY);

        // start http-server
        main_server::HttpServer server("http://0.0.0.0:"+std::to_string(HTTP_SERVER_PORT), input_queue, output_queue);
        server.start();
        std::cout << "Server start" << std::endl;

        // start handler
        main_server::RequestHandler handler(input_queue, download_queue, output_queue);
        handler.start();
        std::cout << "Handler start" << std::endl;

        // start downloader
        main_server::PackageDownloader downloader(download_queue, output_queue);
        downloader.start();
        std::cout << "Downloader start" << std::endl;

        std::cout << "Server running. Press Enter to exit..." << std::endl;
        std::cin.get();

        server.stop();
        handler.stop();
        // downloader.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
