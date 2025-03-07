//
// Created by Kymus-team on 2/22/25.
//
#include "../include/PackageDownloader.h"
#include "../include/LightJson.h"
#include "../include/HeavyJson.h"
#include "../include/DatabaseManager.h"
#include "../include/HttpServer.h"
#include "../include/RequestHandler.h"
#include <iostream>
#include <folly/MPMCQueue.h>
#include <string>

int main() {
    constexpr std::size_t QUEUE_CAPACITY = 1000;

    //DB-init
    main_server::DatabaseManger::init("mongodb://127.0.0.1:<PORT>");

    //Queue-s init
    folly::MPMCQueue<main_server::LightJSON> input_queue(QUEUE_CAPACITY);
    folly::MPMCQueue<main_server::LightJSON> download_queue(QUEUE_CAPACITY);
    folly::MPMCQueue<main_server::HeavyJSON> output_queue(QUEUE_CAPACITY);


    //start http-server
    main_server::HttpServer server("127.0.0.1:<PORT>", input_queue, output_queue);
    server.start();

    //start handeler
    main_server::RequestHandler handler(input_queue, download_queue, output_queue);
    handler.start();

    //start downloader
    //main_server::PackageDownloader downloader();
    //downloader.start();

    std::cout << "Server running. Press Enter to exit..." << std::endl;
    std::cin.get();


    server.stop();
    handler.stop();
    //downloader.stop();
    return 0;
};
