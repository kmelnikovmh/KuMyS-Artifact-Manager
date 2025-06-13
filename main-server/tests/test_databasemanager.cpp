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
    static constexpr const char* TEST_NAME = "test_package_123";

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
        pkg.id = 1;  // numeric request id
        pkg.request_type = "install";
        pkg.name = TEST_NAME;  // package identifier for DB
        pkg.version = "1.0.0";
        pkg.architecture = "amd64";
        pkg.check_sum = "sha256:test123";
        pkg.repo = "test-repo";
        pkg.path = "/test/path";
        pkg.file_size = 1024;
        pkg.content = {0x01, 0x02, 0x03, 0x04};
        pkg.headers = {
            {"Content-Type", "application/json"},
            {"Authorization", "Bearer test-token"}
        };
        pkg.created_at = "2025-01-01";
        return pkg;
    }

private:
    inline static std::unique_ptr<DatabaseManager> manager = nullptr;

    void cleanupTestData() {
        auto conn = folly::coro::blockingWait(
            DatabaseManager::get_connection_async()
        );
        auto db = (*conn)[DatabaseManager::DB_NAME];
        db[TEST_COLLECTION].delete_many({});
        auto bucket = db.gridfs_bucket(
            mongocxx::options::gridfs::bucket{}.bucket_name(TEST_BUCKET)
        );
        for (auto&& file : bucket.find({})) {
            bucket.delete_file(file["_id"].get_value());
        }
    }
};

TEST_F(DatabaseManagerTest, StoreAndRetrievePackage) {
    auto package = createTestPackage();
    // Store under name TEST_NAME
    ASSERT_NO_THROW(
        folly::coro::blockingWait(
            DatabaseManager::store_package(package)
        )
    );

    // Check existence by name
    bool exists = folly::coro::blockingWait(
        DatabaseManager::check_package(package.name)
    );
    ASSERT_TRUE(exists);

    // Fetch by name
    HeavyJSON retrieved;
    ASSERT_NO_THROW(
        retrieved = folly::coro::blockingWait(
            DatabaseManager::fetch_package(package.name)
        )
    );

    // Validate fields
    EXPECT_EQ(retrieved.name, package.name);
    EXPECT_EQ(retrieved.version, package.version);
    EXPECT_EQ(retrieved.architecture, package.architecture);
    EXPECT_EQ(retrieved.check_sum, package.check_sum);
    EXPECT_EQ(retrieved.repo, package.repo);
    EXPECT_EQ(retrieved.path, package.path);
    EXPECT_EQ(retrieved.file_size, package.file_size);
    EXPECT_EQ(retrieved.created_at, package.created_at);
    EXPECT_EQ(retrieved.content, package.content);
    EXPECT_EQ(retrieved.headers, package.headers);
}

TEST_F(DatabaseManagerTest, CheckNonExistentPackage) {
    bool exists = folly::coro::blockingWait(
        DatabaseManager::check_package("nonexistent")
    );
    ASSERT_FALSE(exists);
}

TEST_F(DatabaseManagerTest, FetchNonExistentPackage) {
    EXPECT_THROW(
        folly::coro::blockingWait(
            DatabaseManager::fetch_package("nonexistent")
        ),
        std::runtime_error
    );
}

TEST_F(DatabaseManagerTest, StoreDuplicatePackage) {
    auto pkg = createTestPackage();
    ASSERT_NO_THROW(
        folly::coro::blockingWait(DatabaseManager::store_package(pkg))
    );
    // Storing same name again should throw
    EXPECT_THROW(
        folly::coro::blockingWait(DatabaseManager::store_package(pkg)),
        std::exception
    );
}

TEST_F(DatabaseManagerTest, LargeFileHandling) {
    auto pkg = createTestPackage();
    pkg.content.resize(5 * 1024 * 1024, 0xFF);
    ASSERT_NO_THROW(
        folly::coro::blockingWait(DatabaseManager::store_package(pkg))
    );
    auto retrieved = folly::coro::blockingWait(
        DatabaseManager::fetch_package(pkg.name)
    );
    EXPECT_EQ(retrieved.content.size(), pkg.content.size());
    EXPECT_EQ(retrieved.content.back(), 0xFF);
}

} // namespace test
} // namespace main_server
