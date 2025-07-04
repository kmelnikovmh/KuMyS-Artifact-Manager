cmake_minimum_required(VERSION 3.15)
project(KuMyS-Artifact-Manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/../common-include)
include_directories(${PROJECT_SOURCE_DIR}/include)

# FastCGI
set(FASTCGIPP_INCLUDE_DIR "/usr/include")
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(FASTCGIPP_LIBRARY "/usr/lib/aarch64-linux-gnu/libfastcgipp.so")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(FASTCGIPP_LIBRARY "/usr/lib/x86_64-linux-gnu/libfastcgipp.so")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()
include_directories(${FASTCGIPP_INCLUDE_DIR})

# CppRestSDK
find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Folly
# set(FOLLY_INCLUDE_DIR "/tmp/fbcode_builder_getdeps-ZfollyZbuildZfbcode_builder-root/installed/folly/include")
# set(FOLLY_LIBRARY "/tmp/fbcode_builder_getdeps-ZfollyZbuildZfbcode_builder-root/build/folly")
# include_directories(${FOLLY_INCLUDE_DIR})

# Validate-module
add_executable(validate-module src/validate_request_module.cpp src/main_validate_request_module.cpp)
target_link_libraries(validate-module ${FASTCGIPP_LIBRARY})

# Admin-panel
add_executable(admin-panel src/admin_panel.cpp src/main_admin_panel.cpp)
target_compile_options(admin-panel
    PRIVATE "-pthread"
)
target_link_libraries(admin-panel ${OPENSSL_LIBRARIES} cpprestsdk::cpprest)

# Buffer
add_executable(buffer src/buffer.cpp src/main_buffer.cpp)
target_compile_options(buffer
    PRIVATE  "-pthread"  # Thread safety
)
target_link_libraries(buffer ${OPENSSL_LIBRARIES} cpprestsdk::cpprest)

