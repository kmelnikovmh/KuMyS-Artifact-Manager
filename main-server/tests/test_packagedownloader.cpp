//
// Created by Sergei on 2/22/25.
//
#include <gtest/gtest.h>
#include <folly/MPMCQueue.h>
#include "PackageDownloader.h"
#include <cpprest/http_client.h>
#include <fstream>

using namespace main_server;

const char* TEST_REPOS_FILE = "test_repos.list";

class PackageDownloaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream repo_file(TEST_REPOS_FILE);
        repo_file << "main http://repo1.example.com http://mirror1.example.com\n";
        repo_file << "contrib http://repo2.example.com\n";
        repo_file.close();
    }

    void TearDown() override {
        std::remove(TEST_REPOS_FILE);
    }

    folly::MPMCQueue<LightJSON> download_queue{10};
    folly::MPMCQueue<HeavyJSON> output_queue{10};
};

TEST_F(PackageDownloaderTest, InitializationAndStartStop) {
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    EXPECT_NO_THROW(downloader.start());
    EXPECT_NO_THROW(downloader.stop());
}

TEST_F(PackageDownloaderTest, RepoLoading) {
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    downloader.start();

    const auto& repos = downloader.get_allowed_repos();
    ASSERT_EQ(repos.size(), 2);
    EXPECT_EQ(repos.at("main").proxy_urls[0], "http://repo1.example.com");
    EXPECT_EQ(repos.at("contrib").proxy_urls[0], "http://repo2.example.com");
}

TEST_F(PackageDownloaderTest, UrlGeneration) {
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    downloader.start();
    
    LightJSON test_package{
        "pkg1",
        "install",
        "test-package",
        "1.0",
        "amd64",
        "sha256sum",
        "main",
        "/pool/main/t"
    };

    std::vector<std::string> expected_urls = {
        "http://repo1.example.com/pool/main/t/test-package_1.0_amd64.deb",
        "http://mirror1.example.com/pool/main/t/test-package_1.0_amd64.deb"
    };
    EXPECT_EQ(downloader.generate_urls(test_package), expected_urls);
}

TEST_F(PackageDownloaderTest, FailedRepoLookup) {
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    downloader.start();
    
    LightJSON test_package{
        "pkg1",
        "install",
        "test-package",
        "1.0",
        "amd64",
        "sha256sum",
        "invalid-repo",
        "/pool/main/t"
    };

    testing::internal::CaptureStderr();
    auto urls = downloader.generate_urls(test_package);
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(output.find("Repo invalid-repo is not listed") != std::string::npos);
    EXPECT_TRUE(urls.empty());
}

TEST_F(PackageDownloaderTest, SuccessfulDownload) {
    LightJSON test_package{
        "pkg1",
        "install",
        "test-package",
        "1.0",
        "amd64",
        "sha256sum",
        "main",
        "success"
    };
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    downloader.start();
    downloader.send_request_ = [](const std::string& url, const web::http::http_request& method) {
        web::http::http_response response;
        if (url.find("success") != std::string::npos) {
            response.set_status_code(web::http::status_codes::OK);
            response.set_body(std::vector<unsigned char>{1, 2, 3, 4});
        } else {
            response.set_status_code(web::http::status_codes::NotFound);
        }
        return pplx::task_from_result(response);
    };
    
    download_queue.blockingWrite(std::forward<LightJSON>(test_package));
    
    HeavyJSON result;
    output_queue.blockingRead(result);
    
    EXPECT_EQ(result.name, "test-package");
    EXPECT_EQ(result.content.size(), 4);
    EXPECT_EQ(result.check_sum, "sha256sum");
}

TEST_F(PackageDownloaderTest, FailedDownloadHandling) {
    LightJSON test_package{
        "pkg1",
        "install",
        "test-package",
        "1.0",
        "amd64",
        "sha256sum",
        "main",
        "fail"
    };
    
    PackageDownloader downloader(download_queue, output_queue, TEST_REPOS_FILE);
    downloader.start();
    downloader.send_request_ = [](const std::string& url, const web::http::http_request& method) {
        web::http::http_response response;
        if (url.find("success") != std::string::npos) {
            response.set_status_code(web::http::status_codes::OK);
            response.set_body(std::vector<unsigned char>{1, 2, 3, 4});
        } else {
            response.set_status_code(web::http::status_codes::NotFound);
        }
        return pplx::task_from_result(response);
    };
    
    testing::internal::CaptureStderr();
    download_queue.blockingWrite(std::forward<LightJSON>(test_package));
    
    HeavyJSON result;
    output_queue.blockingRead(result);

    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(output.find("HTTP error: 404") != std::string::npos);
}

TEST_F(PackageDownloaderTest, DatabaseStorage) {
    HeavyJSON test_package{
        "pkg1",
        "install",
        "test-package",

        "1.0",
        "amd64",
        "sha256sum",
        "main",
        "/path",
        4,
        {1,2,3,4},
        "2024-01-01"
    };
    
    bool storage_called = false;
    PackageDownloader downloader(download_queue, output_queue);
    downloader.store_to_database_ = [&storage_called](const HeavyJSON&) {
        storage_called = true;
    };
    
    downloader.store_to_database_(test_package);
    EXPECT_TRUE(storage_called);
}