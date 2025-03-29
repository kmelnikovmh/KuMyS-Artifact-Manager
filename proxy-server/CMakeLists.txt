cmake_minimum_required(VERSION 3.15)
project(KuMyS-Artifact-Manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/../common-include)
include_directories(${PROJECT_SOURCE_DIR}/include)
# FastCGI
set(FASTCGIPP_INCLUDE_DIR "/usr/include")
set(FASTCGIPP_LIBRARY "/usr/lib/aarch64-linux-gnu/libfastcgipp.so")
include_directories(${FASTCGIPP_INCLUDE_DIR})
# CppRestSDK
find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
# Folly
# set(FOLLY_INCLUDE_DIR "/tmp/fbcode_builder_getdeps-ZfollyZbuildZfbcode_builder-root/installed/folly/include")
# set(FOLLY_LIBRARY "/tmp/fbcode_builder_getdeps-ZfollyZbuildZfbcode_builder-root/build/folly")
# include_directories(${FOLLY_INCLUDE_DIR})


add_executable(validate_module src/validate_request_module.cpp)
target_link_libraries(validate_module ${FASTCGIPP_LIBRARY})

add_executable(buffer src/buffer.cpp)
target_compile_options(buffer
    PRIVATE  "-pthread"  # Thread safety
)
target_link_libraries(buffer ${OPENSSL_LIBRARIES} cpprestsdk::cpprest)


