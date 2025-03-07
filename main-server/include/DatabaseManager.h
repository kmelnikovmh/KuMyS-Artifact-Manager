//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
#define KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
#include "HeavyJson.h"
#include <folly/futures/Future.h>
#include <mongocxx/pool.hpp>
#include <string>
#include <folly/experimental/coro/Task.h>

namespace main_server {

class DatabaseManager {
public:
  static void init(const std::string &connection_uri);

  static folly::coro::Task<bool> check_package(std::string &package_id);

  static folly::coro::Task<HeavyJSON> fetch_package(std::string &package_id);
  static folly::coro::Task<void>  store_package(const HeavyJSON &package);

private:
  static inline std::unique_ptr<mongocxx::pool> connection_pool;
};
} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_DATABESMANGER_H
