cmake_minimum_required(VERSION 3.15)
project(KuMyS-Artifact-Manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(main src/main.cpp)
target_link_libraries(tic-tac-toe ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
