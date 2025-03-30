//
// Created by Sergei on 2/22/25.
//
#include <gtest/gtest.h>
#include <DatabaseManager.h>
#include <HeavyJson.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/types.hpp>

namespace main_server {
namespace test {
mongocxx::instance instance{};

class DatabaseManagerTest : public ::testing::Test {
protected:
    static constexpr const char* TEST_DB = "packages_db";
    static constexpr const char* TEST_COLLECTION = "packages";
    static constexpr const char* TEST_BUCKET = "fs";
    static constexpr const char* TEST_ID = "test_package_123";

    void SetUp() override {
        if (!manager) { 
            manager = std::make_unique<DatabaseManager>(
                "mongodb://root:123@mongodb:27017"
            );
        }
        cleanupTestData();
    }

    HeavyJSON createTestPackage() {
        HeavyJSON pkg;
        pkg.id = TEST_ID;
        pkg.request_type = "install";
        pkg.name = "test-package";
        pkg.version = "1.0.0";
        pkg.architecture = "amd64";
        pkg.check_sum = "sha256:test123";
        pkg.file_size = 1024;
        pkg.content = {0x01, 0x02, 0x03, 0x04};
        return pkg;
    }

private:
    inline static std::unique_ptr<DatabaseManager> manager = nullptr;

    void cleanupTestData() {
        auto conn = folly::coro::blockingWait(
            DatabaseManager::get_connection_async()
        );
        
        // Clean collection
        auto db = (*conn)[DatabaseManager::DB_NAME];
        db[TEST_COLLECTION].delete_many({});

        // Clean GridFS
        auto bucket = db.gridfs_bucket(
            mongocxx::options::gridfs::bucket{}.bucket_name(TEST_BUCKET)
        );
        auto files = bucket.find({});
        for (auto&& file : files) {
            bucket.delete_file(file["_id"].get_value());
        }
    }
};

TEST_F(DatabaseManagerTest, StoreAndRetrievePackage) {
    auto package = createTestPackage();
    
    // Test store
    ASSERT_NO_THROW(
        folly::coro::blockingWait(
            DatabaseManager::store_package(package)
        )
    );

    // Test check
    bool exists = folly::coro::blockingWait(
        DatabaseManager::check_package(package.id)
    );
    ASSERT_TRUE(exists);

    // Test fetch
    HeavyJSON retrieved;
    ASSERT_NO_THROW(
        retrieved = folly::coro::blockingWait(
            DatabaseManager::fetch_package(package.id))
    );

    // Validate data
    EXPECT_EQ(retrieved.id, package.id);
    EXPECT_EQ(retrieved.name, package.name);
    EXPECT_EQ(retrieved.version, package.version);
    EXPECT_EQ(retrieved.content, package.content);
}

TEST_F(DatabaseManagerTest, CheckNonExistentPackage) {
    std::string fake_id = "non_existent_package_999";
    bool exists = folly::coro::blockingWait(
        DatabaseManager::check_package(fake_id)
    );
    ASSERT_FALSE(exists);
}

TEST_F(DatabaseManagerTest, FetchNonExistentPackage) {
    std::string fake_id = "non_existent_package_999";
    EXPECT_THROW(
        folly::coro::blockingWait(
            DatabaseManager::fetch_package(fake_id)),
        std::runtime_error
    );
}

TEST_F(DatabaseManagerTest, StoreDuplicatePackage) {
    auto package = createTestPackage();
    
    // First store should succeed
    ASSERT_NO_THROW(
        folly::coro::blockingWait(
            DatabaseManager::store_package(package))
    );

    // Second store should throw
    EXPECT_THROW(
       folly::coro::blockingWait(
           DatabaseManager::store_package(package)),
        std::exception
    );
}

TEST_F(DatabaseManagerTest, LargeFileHandling) {
    HeavyJSON package = createTestPackage();
    package.content.resize(5 * 1024 * 1024, 0xFF); // 5MB
    
    ASSERT_NO_THROW(
        folly::coro::blockingWait(
            DatabaseManager::store_package(package))
    );

    auto retrieved = folly::coro::blockingWait(
        DatabaseManager::fetch_package(package.id)
    );
    
    EXPECT_EQ(retrieved.content.size(), package.content.size());
    EXPECT_EQ(retrieved.content.back(), 0xFF);
}

} // namespace test
} // namespace main_server