//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
#define KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
#include <string>
#include <mongocxx/pool.hpp>
#include <folly/futures/Future.h>
#include "HeavyJson.h"

namespace main_server{

class DatabaseManger {
public:
    static void init(const std::string& connection_uri);


    static folly::Future<bool> check_package_exists(const std::string& package_id);
    static folly::Future<HeavyJSON> fetch_package(const std::string& package_id);
    static folly::Future<void> store_package(const HeavyJSON& package);
private:
    static inline std::unique_ptr<mongocxx::pool> connection_pool;


};
}// namespace main_server

#endif //KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
