//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_LIGHTJSON_H
#define KUMYS_ARTIFACT_MANAGER_LIGHTJSON_H
#include <string>
#include <vector>
#include <cstdint>
namespace main_server {
struct LightJSON {
  uint64_t id;
  std::string request_type;
  std::string name;
  std::string version;
  std::string architecture;
  std::string check_sum;
  std::string repo;
  std::string path;
};

} // namespace main_server
#endif // KUMYS_ARTIFACT_MANAGER_LIGHTJSON_H
