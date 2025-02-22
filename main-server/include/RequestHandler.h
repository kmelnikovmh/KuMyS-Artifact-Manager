//
// Created by Kymus-team on 2/22/25.
//
#ifndef KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
#define KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
#include <folly/futures/Future.h>
#include "LightJson.h"

namespace main_server{

class RequestHandler {
public:
    static folly::Future<LightJSON> validate_package(const std::string& package_id);
    static folly::Future<LightJSON> process_single_package(const std::string& package_id);

};

}// namespace main_server

#endif //KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
