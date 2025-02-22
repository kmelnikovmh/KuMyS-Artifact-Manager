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
)

#Add path to lib
link_directories(
        ${MONGOCXX_LIBRARY_DIR}
        ${BSONCXX_LIBRARY_DIR}
)

# find dependencies
find_package(cpprestsdk REQUIRED)
find_package(folly REQUIRED)


add_executable(kumys_server
        src/main.cpp
        src/HttpServer.cpp
        src/RequestHandler.cpp
        src/DatabaseManager.cpp
        src/PackageDownloader.cpp
        include/DatabaseManager.h
        include/HttpServer.h
        include/RequestHandler.h
        include/DatabaseManager.h
        include/PackageDownloader.h
)

#Include libs
target_link_libraries(kumys_server
        PRIVATE
        cpprestsdk::cpprest
        mongocxx
        bsoncxx
        Folly::folly
)