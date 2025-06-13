//
// Created by Kymus-team on 2/22/25.
//
#include <gtest/gtest.h>
#include <folly/MPMCQueue.h>
#include <thread>
#include <chrono>

#include "RequestHandler.h"
#include "LightJson.h"
#include "HeavyJson.h"

using namespace main_server;
TEST(RequestHandlerTest, NewPackageGoesToDownloadQueue) {
    folly::MPMCQueue<LightJSON> inq(10), dq(10);
    folly::MPMCQueue<HeavyJSON> oq(10);
    RequestHandler handler(inq, dq, oq);
    handler.start();

    LightJSON pkg;
    pkg.id = 1;
    pkg.name = "new-pkg";
    pkg.request_type = "install";
    pkg.version = "1.0";
    pkg.architecture = "amd64";
    pkg.check_sum = "sha256:abc";
    pkg.repo = "test_repo";
    pkg.path = "/path";

    inq.blockingWrite(std::move(pkg));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LightJSON result;
    ASSERT_TRUE(dq.tryReadUntil(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(100), result
    ));
    EXPECT_EQ(result.name, "new-pkg");

    handler.stop();
}

TEST(RequestHandlerTest, MultiplePackagesProcessed) {
    folly::MPMCQueue<LightJSON> inq(10), dq(10);
    folly::MPMCQueue<HeavyJSON> oq(10);
    RequestHandler handler(inq, dq, oq);
    handler.start();

    const int N = 3;
    for (int i = 0; i < N; ++i) {
        LightJSON pkg;
        pkg.id = i;
        pkg.name = "pkg" + std::to_string(i);
        pkg.request_type = "install";
        pkg.version = "1.0";
        pkg.architecture = "amd64";
        pkg.check_sum = "sha256";
        pkg.repo = "test_repo";
        pkg.path = "/path";
        inq.blockingWrite(std::move(pkg));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int count = 0;
    LightJSON out;
    while (dq.tryReadUntil(
        std::chrono::steady_clock::now() + std::chrono::milliseconds(10), out)) {
        ++count;
    }
    EXPECT_EQ(count, N);

    handler.stop();
}
