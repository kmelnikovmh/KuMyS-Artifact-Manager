//
// Created by Kymus-team on 2/22/25.
//

#include "../include/RequestHandler.h"
#include <folly/futures/Future.h>
#include "../include/DatabaseManager.h"
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <iostream>
#include <memory>
#include <utility>

#include <folly/experimental/coro/BlockingWait.h>



main_server::RequestHandler::RequestHandler(
        folly::MPMCQueue<LightJSON> &input_queue,
        folly::MPMCQueue<LightJSON> &download_queue,
        folly::MPMCQueue<HeavyJSON> &output_queue)
        : input_queue_(input_queue), download_queue_(download_queue),
          output_queue_(output_queue) {

}

void main_server::RequestHandler::start() {
    stopped_.store(false);
    worker_ = std::thread([this] { processLoop(); });
}


void main_server::RequestHandler::stop() {
    stopped_.store(true);
    executor_->join();
    if (worker_.joinable()) worker_.join();
}

void main_server::RequestHandler::processLoop() {
    while (!stopped_.load()) {
        LightJSON package;
        if (input_queue_.readIfNotEmpty(package)) {
            folly::coro::co_invoke([this, pkg = std::move(package)] {
                return processPackage(std::move(pkg));
            }).scheduleOn(executor_.get()).start();
        } else {
            std::this_thread::yield();
        }

    }
}

folly::coro::Task<void> main_server::RequestHandler::processPackage(main_server::LightJSON package) {
    try {
        bool exist = co_await main_server::DatabaseManager::check_package(package.id);
        if (exist) {
            HeavyJSON heavyJSON = co_await main_server::DatabaseManager::fetch_package(package.id);
            output_queue_.blockingWrite(std::move(heavyJSON));
        } else {
            download_queue_.blockingWrite(std::move(package));
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

