include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

add_executable(tests
        test_main.cpp
        test_httpserver.cpp
        test_requesthandler.cpp
        test_databasemanager.cpp
        test_packagedownloader.cpp
)

target_link_libraries(tests
        PRIVATE
        kumys_lib
        cpprestsdk::cpprest
        mongocxx
        bsoncxx
        Folly::folly
        gtest gmock gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
