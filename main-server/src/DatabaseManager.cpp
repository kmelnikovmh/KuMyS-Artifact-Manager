//
// Created by Kymus-team on 2/22/25.
//

#include "../include/DatabaseManager.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <iostream>
#include <tclDecls.h>
// TODO

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

static std::shared_ptr<folly::CPUThreadPoolExecutor> db_executor;

void main_server::DatabaseManager::init(const std::string& connection_uri) {
    if (!connection_pool_) {
        mongocxx::uri uri(connection_uri);
        connection_pool_ = std::make_unique<mongocxx::pool>(uri);
        db_executor = std::make_shared<folly::CPUThreadPoolExecutor>(4);
        auto conn = connection_pool_->acquire();
        auto collection = (*conn)[DB_NAME][COLLECTION_NAME];

        collection.create_index(
                bsoncxx::builder::stream::document{}
                        << "_id" << 1
                        << bsoncxx::builder::stream::finalize,
                mongocxx::options::index{}.unique(true)
        );

    }
}

folly::coro::Task<bool> main_server::DatabaseManager::check_package(std::string& package_id) {
        try {
            auto conn = get_connection();
            auto collection = (*conn)[DB_NAME][COLLECTION_NAME];
            co_return static_cast<bool>(collection.find_one(make_document(kvp("_id", package_id))));
        } catch (const std::exception& e) {
            std::cerr << "Check package error: " << e.what() << std::endl;
            co_return  false;
        }
}

folly::coro::Task<main_server::HeavyJSON> main_server::DatabaseManager::fetch_package(std::string& package_id) {
        try {
            auto conn = get_connection();
            auto collection = (*conn)[DB_NAME][COLLECTION_NAME];

            auto filter = bsoncxx::builder::stream::document{}
                    << "_id" << package_id
                    << bsoncxx::builder::stream::finalize;

            auto data = collection.find_one(filter.view());
            HeavyJSON package = toHeavyJSON(bsoncxx::to_json(*data));

            co_return package;

        } catch (const std::exception& e) {
            std::cerr << "Fetch package error: " << e.what() << std::endl;
            throw;
        }


}

folly::coro::Task<void> main_server::DatabaseManager::store_package(const HeavyJSON& package) {
        try {
            auto conn = get_connection();
            auto collection = (*conn)[DB_NAME][COLLECTION_NAME];

            using namespace bsoncxx::builder;
            stream::document builder;

            builder
                    << "_id" << package.id
                    << "request_type" << package.request_type
                    << "name" << package.name
                    << "version" << package.version
                    << "architecture" << package.architecture
                    << "check_sum" << package.check_sum
                    << "file_size" << static_cast<int64_t>(package.file_size)
                    << "content" << bsoncxx::types::b_binary{
                    bsoncxx::binary_sub_type::k_binary,
                    static_cast<uint32_t>(package.content.size()),
                    package.content.data()
                    }
                    << "created_at" << package.created_at;

            collection.insert_one(builder.view());
        } catch (const std::exception& e) {
            std::cerr << "Store package error: " << e.what() << std::endl;
            throw;
        }
}

mongocxx::pool::entry main_server::DatabaseManager::get_connection() {
    return connection_pool_->acquire();
}
