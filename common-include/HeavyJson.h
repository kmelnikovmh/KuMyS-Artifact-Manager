//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H
#define KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <bsoncxx/types/bson_value/view.hpp>

namespace main_server {
    struct HeavyJSON {
        uint64_t id;
        std::string request_type;
        std::string name;
        std::string version;
        std::string architecture;
        std::string check_sum;
        std::string repo;
        std::string path;

        uint64_t file_size;
        std::vector<uint8_t> content;
        std::string created_at;
        std::unordered_map<std::string, std::string> headers;

        bsoncxx::types::bson_value::view file_id;
    };

} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H