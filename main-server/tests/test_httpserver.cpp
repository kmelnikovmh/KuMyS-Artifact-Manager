#include <gtest/gtest.h>
#include <HttpServer.h>
#include <folly/MPMCQueue.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <gmock/gmock.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace testing;

namespace main_server {
namespace test {

class MockHttpClient {
public:
    MOCK_METHOD(pplx::task<http_response>, Request, (const http_request&));
};

class HttpServerTest : public ::testing::Test {
protected:
    const std::string test_url = "http://localhost:8080";
    folly::MPMCQueue<LightJSON> input_queue{10};
    folly::MPMCQueue<HeavyJSON> output_queue{10};
    
    void SetUp() override {
        server = std::make_unique<HttpServer>(test_url, input_queue, output_queue);
    }

    std::unique_ptr<HttpServer> server;
    MockHttpClient mock_client;
};

TEST_F(HttpServerTest, ResponseRequestSendsCorrectData) {
    HeavyJSON test_package;
    test_package.id = 123;
    test_package.request_type = "install";
    test_package.name = "test-pkg";
    test_package.version = "1.0";
    test_package.architecture = "amd64";
    test_package.check_sum = "sha256:test";
    test_package.file_size = 1024;
    test_package.created_at = "2024-01-01";
    test_package.content = {0x01, 0x02, 0x03};

    class TestableHttpServer : public HttpServer {
    public:
        using HttpServer::HttpServer;
        using HttpServer::response_request;
        using HttpServer::validate_light_json;
    };
    
    auto test_server = std::make_unique<TestableHttpServer>(test_url, input_queue, output_queue);
    
    output_queue.blockingWrite(std::move(test_package));
}

TEST_F(HttpServerTest, HandlePostRequestWithEmptyBody) {
    http_request request(methods::POST);
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000), received));
}

TEST_F(HttpServerTest, HandleMalformedJson) {
    http_request request(methods::POST);
    request.set_body("invalid json {");
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000), received));
}

TEST_F(HttpServerTest, HandleMissingRequiredFields) {
    json::value invalid_body;
    invalid_body[U("id")] = json::value::string("123");
    
    http_request request(methods::POST);
    request.set_body(invalid_body);
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000), received));
}

TEST_F(HttpServerTest, HandleInvalidFieldTypes) {
    json::value invalid_body;
    invalid_body[U("id")] = json::value::number(123);
    invalid_body[U("request_type")] = json::value::string("install");
    invalid_body[U("name")] = json::value::string("test-pkg");
    invalid_body[U("version")] = json::value::string("1.0");
    invalid_body[U("architecture")] = json::value::string("amd64");
    invalid_body[U("check_sum")] = json::value::string("sha256:test");
    invalid_body[U("repo")] = json::value::string("test_repo");
    invalid_body[U("path")] = json::value::string("path/to/package");

    http_request request(methods::POST);
    request.set_body(invalid_body);
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000), received));
}

TEST_F(HttpServerTest, HandleEmptyFieldValues) {
    json::value invalid_body;
    invalid_body[U("id")] = json::value::string("");
    invalid_body[U("request_type")] = json::value::string("install");
    invalid_body[U("name")] = json::value::string("test-pkg");
    invalid_body[U("version")] = json::value::string("1.0");
    invalid_body[U("architecture")] = json::value::string("amd64");
    invalid_body[U("check_sum")] = json::value::string("sha256:test");
    invalid_body[U("repo")] = json::value::string("test_repo");
    invalid_body[U("path")] = json::value::string("path/to/package");

    http_request request(methods::POST);
    request.set_body(invalid_body);
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_FALSE(server->validate_light_json(received));
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(1000), received));
}

TEST_F(HttpServerTest, HandleVeryLargeRequest) {
    // Генерируем большой JSON (>1MB)
    json::value large_body;
    std::string large_string(2*1024*1024, 'a'); // 2MB
    large_body[U("id")] = json::value::string(large_string);
    large_body[U("request_type")] = json::value::string("install");
    large_body[U("name")] = json::value::string("test-pkg");
    large_body[U("version")] = json::value::string("1.0");
    large_body[U("architecture")] = json::value::string("amd64");
    large_body[U("check_sum")] = json::value::string("sha256:test");
    large_body[U("repo")] = json::value::string("test_repo");
    large_body[U("path")] = json::value::string("path/to/package");

    http_request request(methods::POST);
    request.set_body(large_body);
    
    server->handle_post_request(request);
    
    LightJSON received;
    EXPECT_TRUE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(5000), received));
}

TEST_F(HttpServerTest, NumericIdRejected) {
    web::json::value body;
    body[U("id")] = json::value::number(123);
    body[U("request_type")] = json::value::string("install");
    body[U("name")] = json::value::string("pkg");
    body[U("version")] = json::value::string("1.0");
    body[U("architecture")] = json::value::string("amd64");
    body[U("check_sum")] = json::value::string("sha256:abc");
    body[U("repo")] = json::value::string("test_repo");
    body[U("path")] = json::value::string("/path/pkg.deb");

    http_request req(methods::POST);
    req.set_body(body);
    server->handle_post_request(req);

    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(std::chrono::steady_clock::now() + std::chrono::milliseconds(100), received));
}

TEST_F(HttpServerTest, HandleValidPostRequest) {
    json::value body;
    body[U("id")]            = json::value::number(123);
    body[U("request_type")]  = json::value::string("install");
    body[U("name")]          = json::value::string("test-pkg");
    body[U("version")]       = json::value::string("1.2");
    body[U("architecture")]  = json::value::string("amd64");
    body[U("check_sum")]     = json::value::string("sha256:test");
    body[U("repo")]          = json::value::string("test_repo");
    body[U("path")]          = json::value::string("path/to/package");

    http_request req(methods::POST);
    req.set_body(body);
    server->handle_post_request(req);

    LightJSON received;
    ASSERT_TRUE(input_queue.tryReadUntil(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
        received
    ));
    EXPECT_EQ(received.id, 123);
}

TEST_F(HttpServerTest, IgnoresExtraFields) {
    json::value body;
    body[U("id")]            = json::value::number(456);
    body[U("request_type")]  = json::value::string("install");
    body[U("name")]          = json::value::string("pkg");
    body[U("version")]       = json::value::string("1.0");
    body[U("architecture")]  = json::value::string("amd64");
    body[U("check_sum")]     = json::value::string("sha256:abc");
    body[U("repo")]          = json::value::string("test_repo");
    body[U("path")]          = json::value::string("/path/pkg.deb");
    body[U("extra")]         = json::value::string("should_be_ignored");

    http_request req(methods::POST);
    req.set_body(body);
    server->handle_post_request(req);

    LightJSON received;
    ASSERT_TRUE(input_queue.tryReadUntil(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
        received
    ));
    EXPECT_EQ(received.id, 456);
}

TEST_F(HttpServerTest, StringIdRejected) {
    json::value body;
    body[U("id")]            = json::value::string("123");
    body[U("request_type")]  = json::value::string("install");
    body[U("name")]          = json::value::string("test-pkg");
    body[U("version")]       = json::value::string("1.0");
    body[U("architecture")]  = json::value::string("amd64");
    body[U("check_sum")]     = json::value::string("sha256:test");
    body[U("repo")]          = json::value::string("test_repo");
    body[U("path")]          = json::value::string("path/to/package");

    http_request req(methods::POST);
    req.set_body(body);
    server->handle_post_request(req);

    LightJSON received;
    EXPECT_FALSE(input_queue.tryReadUntil(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100),
        received
    ));
}

} // namespace test
} // namespace main_server
