cmake_minimum_required(VERSION 3.15)
project(KuMyS-Artifact-Manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#In my mac Troubles Fix
set(MONGOCXX_INCLUDE_DIR "/usr/local/include/mongocxx/v_noabi")
set(BSONCXX_INCLUDE_DIR "/usr/local/include/bsoncxx/v_noabi")
set(MONGOCXX_LIBRARY_DIR "/usr/local/lib")
set(BSONCXX_LIBRARY_DIR "/usr/local/lib")

#Add path
include_directories(
        ${MONGOCXX_INCLUDE_DIR}
        ${BSONCXX_INCLUDE_DIR}
        include
        ${PROJECT_SOURCE_DIR}/../common-include
)

#Add path to lib
link_directories(
        ${MONGOCXX_LIBRARY_DIR}
        ${BSONCXX_LIBRARY_DIR}
)

# find dependencies
find_package(cpprestsdk REQUIRED)
find_package(folly REQUIRED)
find_package(GFlags REQUIRED)


add_library(kumys_lib STATIC
        src/HttpServer.cpp
        src/RequestHandler.cpp
        src/DatabaseManager.cpp
        src/PackageDownloader.cpp
        src/ServerApp.cpp
        include/DatabaseManager.h
        include/HttpServer.h
        include/RequestHandler.h
        include/DatabaseManager.h
        include/PackageDownloader.h
        include/ServerApp.h
)


#Include libs
target_link_libraries(kumys_lib
        PRIVATE
        cpprestsdk::cpprest
        mongocxx
        bsoncxx
        Folly::folly
)

add_executable(kumys_server src/main.cpp)
target_link_libraries(kumys_server PRIVATE kumys_lib)

#Include tests
# enable_testing()
# add_subdirectory(tests)