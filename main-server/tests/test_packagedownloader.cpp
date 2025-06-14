// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
// #include "PackageDownloader.h"

// using namespace main_server;
// using namespace testing;

// class PackageDownloaderTest : public Test {
// protected:
//     folly::MPMCQueue<LightJSON> download_queue{10};
//     folly::MPMCQueue<HeavyJSON> output_queue{10};
//     std::unique_ptr<PackageDownloader> downloader;

//     void SetUp() override {
//         downloader = std::make_unique<PackageDownloader>(download_queue, output_queue, "test_repos.list");
//     }

//     void TearDown() override {
//         std::remove("test_repos.list");
//     }
// };

// TEST_F(PackageDownloaderTest, GenerateUrlsWithHttpProxy) {
//     Repo repo{"test_repo", "", {"example.com", "https://secure.com"}};
//     downloader->allowed_repositories_["test_repo"] = repo;
    
//     LightJSON pkg;
//     pkg.repo = "test_repo";
//     pkg.path = "pool/main";
//     pkg.name = "test-pkg";
//     pkg.version = "1.0.0";
//     pkg.architecture = "amd64";

//     auto urls = downloader->generate_urls(pkg);
//     ASSERT_EQ(urls.size(), 2);
//     EXPECT_EQ(urls[0], "http://example.com/pool/main/test-pkg_1.0.0_amd64.deb");
//     EXPECT_EQ(urls[1], "https://secure.com/pool/main/test-pkg_1.0.0_amd64.deb");
// }

// TEST_F(PackageDownloaderTest, GenerateUrlsWithInvalidRepo) {
//     LightJSON pkg;
//     pkg.repo = "invalid_repo";
//     auto urls = downloader->generate_urls(pkg);
//     EXPECT_TRUE(urls.empty());
// }

// TEST_F(PackageDownloaderTest, UpdateReposFromConfigFile) {
//     std::ofstream cfg("test_repos.list");
//     cfg << "# Comment\n"
//         << "repo1 http://repo1.local ftp://repo1-backup.local\n"
//         << "repo2 https://repo2.example\n";
//     cfg.close();

//     downloader->update_repos();
//     const auto& repos = downloader->get_allowed_repos();
    
//     ASSERT_EQ(repos.size(), 2);
//     EXPECT_THAT(repos.at("repo1").proxy_urls, ElementsAre("http://repo1.local", "ftp://repo1-backup.local"));
//     EXPECT_THAT(repos.at("repo2").proxy_urls, ElementsAre("https://repo2.example"));
// }

// using namespace main_server;
// using namespace testing;
// using namespace folly::coro;

// class MockPackageDownloader : public PackageDownloader {
// public:
//     using PackageDownloader::PackageDownloader;
    
//     MOCK_METHOD((folly::coro::Task<web::http::http_response>), send_request, (
//         const std::string&, 
//         const web::http::method&
//     ));
    
//     MOCK_METHOD((folly::coro::Task<void>), store_to_database, (const HeavyJSON&));
    
//     // Перенаправляем вызовы функторов в мок-методы
//     MockPackageDownloader(folly::MPMCQueue<LightJSON>& dq, 
//                          folly::MPMCQueue<HeavyJSON>& oq,
//                          std::string cfg = "")
//         : PackageDownloader(dq, oq, cfg) 
//     {
//         store_to_database_ = [this](const HeavyJSON& json) {
//             return store_to_database(json);
//         };
//         send_request_ = [this](const std::string& url, const web::http::method& m) {
//             return send_request(url, m);
//         };
//     }
// };

// TEST_F(PackageDownloaderTest, GenerateUrlsWithHttpProxy) {
//     Repo repo{"test_repo", "", {"example.com", "https://secure.com"}};
//     downloader->allowed_repositories_["test_repo"] = repo;
    
//     LightJSON pkg;
//     pkg.repo = "test_repo";
//     pkg.path = "pool/main";
//     pkg.name = "test-pkg";
//     pkg.version = "1.0.0";
//     pkg.architecture = "amd64";

//     auto urls = downloader->generate_urls(pkg);
//     ASSERT_EQ(urls.size(), 2);
//     EXPECT_EQ(urls[0], "http://example.com/pool/main/test-pkg_1.0.0_amd64.deb");
//     EXPECT_EQ(urls[1], "https://secure.com/pool/main/test-pkg_1.0.0_amd64.deb");
// }

// TEST_F(PackageDownloaderTest, SuccessDownloadAndStore) {
//     LightJSON pkg;
//     pkg.repo = "test_repo";
//     pkg.request_type = "install";
    
//     web::http::http_response response;
//     response.set_status_code(web::http::status_codes::OK);
//     response.set_body("test_data");
    
//     EXPECT_CALL(*downloader, send_request(_, _))
//         .WillOnce(Return(folly::coro::makeTask(response)));
//     EXPECT_CALL(*downloader, store_to_database(_))
//         .WillOnce(Return(folly::coro::makeTask()));
    
//     blockingWait(downloader->download_package(pkg));
    
//     HeavyJSON result;
//     ASSERT_TRUE(output_queue.tryRead(result));
//     EXPECT_EQ(result.content, "test_data"); // Предполагая, что поле называется content
// }

// TEST_F(PackageDownloaderTest, ProcessLoopIntegration) {
//     downloader->start();
    
//     LightJSON pkg;
//     pkg.repo = "test_repo";
//     download_queue.blockingWrite(pkg);
    
//     HeavyJSON result;
//     ASSERT_TRUE(output_queue.read(result)); // Используем блокирующее чтение
//     downloader->stop();
// }
