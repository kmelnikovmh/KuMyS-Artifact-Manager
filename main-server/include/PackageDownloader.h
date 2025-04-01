//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#define KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#include "LightJson.h"
#include <folly/futures/Future.h>
#include <string>

namespace main_server {

class PackageDownloader {
public:
  static folly::Future<LightJSON>
  download_single_package(const std::string &package_id);

  // TODO @mixalowstonks
};
} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
