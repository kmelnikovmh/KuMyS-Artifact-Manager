//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H
#define KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H
#include <string>
#include <vector>

namespace main_server{
    struct HeavyJSON{
        std::string id;
        std::string request_type;
        std::string name;
        std::string version;
        std::string architecture;
        std::string check_sum;

        std::size_t file_size;
        std::string content; //TODO NORMAL TYPE
        std::string created_at;
    };

}// namespace main_server

#endif //KUMYS_ARTIFACT_MANAGER_HEAVYJSON_H
