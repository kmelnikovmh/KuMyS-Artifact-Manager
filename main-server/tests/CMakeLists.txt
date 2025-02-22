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
        GTest::GTest
        GMock::GMock
)

include(GoogleTest)
gtest_discover_tests(tests)