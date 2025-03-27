//
// Created by Kymus-team on 2/22/25.
//

#include "PackageDownloader.h"
#include <folly/futures/Future.h>
#include <folly/MPMCQueue.h>
#include "DatabaseManager.h"
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Sleep.h>
#include <cpprest/uri_builder.h>
#include <cpprest/http_client.h>
#include <iostream>
#include <memory>
#include <utility>
    
main_server::PackageDownloader::PackageDownloader(
    folly::MPMCQueue<LightJSON> &download_queue,
    folly::MPMCQueue<HeavyJSON> &output_queue)
    : download_queue_(download_queue),
      output_queue_(output_queue),
      executor_(std::thread::hardware_concurrency()) {

}

void main_server::PackageDownloader::start() {
    is_running_.store(true);
    worker_ = std::thread([this]() { process_loop(); }); 
    worker_.detach();
}


void main_server::PackageDownloader::stop() {
    is_running_.store(false);
    executor_.join();
    if (worker_.joinable()) worker_.join();
}

void main_server::PackageDownloader::process_loop() {
    LightJSON package;
    while(is_running_.load()) {
        download_queue_.blockingRead(package);
        executor_.add([this, package = std::move(package)]() mutable {
            download_package(package);
        });
    }
}

void main_server::PackageDownloader::download_package(const LightJSON& package) {
    web::uri_builder builder;
    builder.set_scheme(U("https"))
        .set_host(U(package.repo))
        .append_path(U(package.path))
        .append_path(U(package.name + "_" + package.version + "_" + package.architecture + ".deb"));
    
    const std::string url = builder.to_string();
    
    web::http::client::http_client client(U(url));
    client.request(web::http::methods::GET)
        .then([this, package](const web::http::http_response& response) {
            if (response.status_code() == web::http::status_codes::OK) {
                return response.extract_vector();
            } else {
                std::cerr << "HTTP STATUS" << response.status_code() << std::endl;
            }
        }).then([this, package](const std::vector<unsigned char>& data) {
            HeavyJSON downloaded_package{
                package.id,
                package.request_type,
                package.name,
                package.version,
                package.architecture,
                package.check_sum,
                package.repo,
                package.path,
                data.size(), // ? I guess this is wrong
                data,
                utility::datetime::utc_now().to_string()
            };
            store_to_database(downloaded_package);
        }).then([]() {
            // logging
            return pplx::task_from_result();
        }).wait();
}

void main_server::PackageDownloader::store_to_database(const HeavyJSON& package) {
    main_server::DatabaseManger::store_package(package);
}

