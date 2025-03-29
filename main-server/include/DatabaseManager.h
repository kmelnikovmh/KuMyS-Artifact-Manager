//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
#define KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H

#include "HeavyJson.h"
#include <mongocxx/pool.hpp>
#include <string>
#include <folly/experimental/coro/Task.h>

namespace main_server {

    class DatabaseManager {
    public:
        explicit DatabaseManager(const std::string &connection_uri);
        static folly::coro::Task<bool> check_package(std::string &package_id);

        static folly::coro::Task<HeavyJSON> fetch_package(std::string &package_id);

        static folly::coro::Task<void> store_package(const HeavyJSON &package);

    // private:
        static std::unique_ptr<mongocxx::pool> connection_pool_;
        static std::unique_ptr<mongocxx::gridfs::bucket> gridfs_bucket_;
        static std::atomic<bool> initialized_;
        static std::mutex init_mutex_;

        static inline const std::string DB_NAME = "packages_db";
        static inline const std::string COLLECTION_NAME = "packages";

        // Вспомогательный метод для получения соединения
        static folly::coro::Task<mongocxx::pool::entry> get_connection_async();
        static constexpr const char* BUCKET_NAME = "fs";



    };
} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
