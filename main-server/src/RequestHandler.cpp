//
// Created by Kymus-team on 2/22/25.
//

#include "../include/RequestHandler.h"
#include <folly/futures/Future.h>
#include "../include/DatabaseManager.h"
#include <folly/experimental/coro/Sleep.h>
#include <iostream>

#include <folly/experimental/coro/BlockingWait.h>


// TODO
main_server::RequestHandler::RequestHandler(
    folly::MPMCQueue<LightJSON> &input_queue,
    folly::MPMCQueue<LightJSON> &download_queue,
    folly::MPMCQueue<HeavyJSON> &output_queue,
    folly::Executor::KeepAlive<> executor)
    : input_queue_(input_queue), download_queue_(download_queue),
      output_queue_(output_queue), executor_(executor) {

}

void main_server::RequestHandler::start() {
    tasks_.push_back(processLoop().scheduleOn(executor_).start());
}


void main_server::RequestHandler::stop() {

}

folly::coro::Task<void> main_server::RequestHandler::processLoop() {
    while(true){
        LightJSON package;
        if(input_queue_.tryRead()){
            co_await processPackage(std::move(package.value()));
        }
        else{
            co_await folly::coro::sleep(std::chrono::milliseconds(10));
        }

    }
}

folly::coro::Task<void> main_server::RequestHandler::processPackage(main_server::LightJSON package) {
    try{
        bool exist = co_await main_server::DatabaseManager::check_package(package.id);
        if(exist){
            HeavyJSON heavyJSON = co_await main_server::DatabaseManager::fetch_package(package.id);

            co_await output_queue_.blockingWrite(std::move(heavyJSON));
        }
        else{
            co_await download_queue_.blockingWrite(std::move(package));
        }
    }
    catch (const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

