//
// Created by Kymus-team on 2/22/25.
//

#include "../include/DatabaseManager.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <folly/experimental/coro/BlockingWait.h>
#include <iostream>
#include <mongocxx/exception/gridfs_exception.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using namespace bsoncxx::builder;

std::unique_ptr<mongocxx::pool>           main_server::DatabaseManager::connection_pool_ = nullptr;
std::unique_ptr<mongocxx::gridfs::bucket> main_server::DatabaseManager::gridfs_bucket_   = nullptr;

std::atomic<bool> main_server::DatabaseManager::initialized_{false};
std::mutex        main_server::DatabaseManager::init_mutex_;

main_server::DatabaseManager::DatabaseManager(const std::string& connection_uri) {

    std::lock_guard<std::mutex> lock(init_mutex_);
    if (!initialized_.load(std::memory_order_relaxed)) {
        mongocxx::uri uri(connection_uri);

        connection_pool_ = std::make_unique<mongocxx::pool>(uri);

        auto conn = connection_pool_->acquire();
        auto db   = (*conn)[DB_NAME];

        mongocxx::options::gridfs::bucket bucket_options;
        bucket_options.bucket_name(BUCKET_NAME);
        bucket_options.chunk_size_bytes(255 * 1024); // 255KB

        gridfs_bucket_ = std::make_unique<mongocxx::gridfs::bucket>(db.gridfs_bucket(bucket_options));

        db[COLLECTION_NAME].create_index(bsoncxx::builder::stream::document{} << "id" << 1
                                                                              << bsoncxx::builder::stream::finalize,
                                         mongocxx::options::index{}.unique(true));

        initialized_.store(true, std::memory_order_release);
    }
}

folly::coro::Task<bool> main_server::DatabaseManager::check_package(std::string package_name) {
    auto conn = co_await get_connection_async();
    try {
        auto collection = (*conn)[DB_NAME][COLLECTION_NAME];
        co_return static_cast<bool>(collection.find_one(make_document(kvp("_id", package_name))));
    } catch (const std::exception& e) {
        std::cerr << "Check package error: " << e.what() << std::endl;
        co_return false;
    }
}

folly::coro::Task<main_server::HeavyJSON> main_server::DatabaseManager::fetch_package(std::string package_name) {
    auto      conn = co_await get_connection_async();
    HeavyJSON package;

    try {
        auto collection = (*conn)[DB_NAME][COLLECTION_NAME];
        auto doc        = collection.find_one(make_document(kvp("_id", package_name)));

        if (!doc)
            throw std::runtime_error("Package not found");

        auto view = doc->view();

        package.id           = static_cast<uint64_t>(view["file_size"].get_int64().value);
        package.request_type = std::string(view["request_type"].get_string().value);
        package.name         = std::string(view["name"].get_string().value);
        package.version      = std::string(view["version"].get_string().value);
        package.architecture = std::string(view["architecture"].get_string().value);
        package.check_sum    = std::string(view["check_sum"].get_string().value);
        package.repo         = std::string(view["repo"].get_string().value);
        package.path         = std::string(view["path"].get_string().value);
        package.file_size    = static_cast<uint64_t>(view["file_size"].get_int64().value);
        package.created_at   = std::string(view["created_at"].get_string().value);

        if (view["headers"]) {
            auto headers_view = view["headers"].get_document().view();
            for (const auto& element : headers_view) {
                package.headers[std::string(element.key())] = std::string(element.get_string().value);
            }
        }

        auto file_id    = view["file_id"].get_value();
        auto downloader = gridfs_bucket_->open_download_stream(file_id);
        package.content.resize(downloader.file_length());

        size_t           total_read     = 0;
        constexpr size_t yield_interval = 4 * 1024 * 1024; // 4MB

        while (total_read < downloader.file_length()) {
            const size_t bytes_read =
                downloader.read(package.content.data() + total_read, downloader.file_length() - total_read);

            total_read += bytes_read;

            if (total_read % yield_interval == 0) {
                co_await folly::coro::co_reschedule_on_current_executor;
            }
        }

        co_return package;

    } catch (const bsoncxx::exception& e) {
        std::cerr << "BSON error: " << e.what() << std::endl;
        throw;
    } catch (const mongocxx::gridfs_exception& e) {
        std::cerr << "GridFS error[" << e.code().value() << "]: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Fetch package error: " << e.what() << std::endl;
        throw;
    }
}

void main_server::DatabaseManager::clean() {
    auto conn = folly::coro::blockingWait(
        DatabaseManager::get_connection_async()
    );
    
    // Clean collection
    auto db = (*conn)[DatabaseManager::DB_NAME];
    db[COLLECTION_NAME].delete_many({});

    // Clean GridFS
    auto bucket = db.gridfs_bucket(
        mongocxx::options::gridfs::bucket{}.bucket_name("fs")
    );
    auto files = bucket.find({});
    for (auto&& file : files) {
        bucket.delete_file(file["_id"].get_value());
    }
    std::cout << "cleaned" << std::endl;
}

folly::coro::Task<void> main_server::DatabaseManager::store_package(const HeavyJSON& package) {
    
    auto conn = co_await get_connection_async();
    try {

        mongocxx::options::gridfs::upload options{};
        options.metadata(make_document(kvp("version", package.version), kvp("architecture", package.architecture)));

        auto uploader = gridfs_bucket_->open_upload_stream(package.name, // filename
                                                           options     // options
        );

        const size_t chunk_size = 255 * 1024; // 255KB
        size_t       offset     = 0;

        while (offset < package.content.size()) {
            const size_t bytes_to_write = std::min(chunk_size, package.content.size() - offset);

            uploader.write(package.content.data() + offset, bytes_to_write);
            offset += bytes_to_write;

            if ((offset % (4 * 1024 * 1024)) == 0) {
                co_await folly::coro::co_reschedule_on_current_executor;
            }
        }

        auto result = uploader.close();

        auto headers_doc = bsoncxx::builder::basic::document{};
        for (const auto& [key, value] : package.headers) {
            headers_doc.append(kvp(key, value));
        }

        auto collection = (*conn)[DB_NAME][COLLECTION_NAME];
        auto doc        = make_document(kvp("_id", package.name),
                                 kvp("request_type", package.request_type),
                                 kvp("name", package.name),
                                 kvp("version", package.version),
                                 kvp("architecture", package.architecture),
                                 kvp("check_sum", package.check_sum),
                                 kvp("repo", package.repo),
                                 kvp("path", package.path),
                                 kvp("file_size", static_cast<int64_t>(package.file_size)),
                                 kvp("file_id", result.id()),
                                 kvp("created_at", package.created_at),
                                 kvp("headers", headers_doc.extract()));

        collection.insert_one(doc.view());
        std::cout << "kinda stored" << std::endl;
    } catch (const mongocxx::gridfs_exception& e) {
        std::cerr << "GridFS Error[" << e.code().value() << "]: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Store package error: " << e.what() << std::endl;
        throw;
    }
}

folly::coro::Task<mongocxx::pool::entry> main_server::DatabaseManager::get_connection_async() {
    co_return connection_pool_->acquire();
}
