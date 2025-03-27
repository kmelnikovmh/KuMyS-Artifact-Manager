//
// Created by Kymus-team on 2/22/25.
//

#ifndef KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#define KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
#include "HeavyJson.h"
#include "LightJson.h"
#include <folly/MPMCQueue.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <string>
#include <thread>

namespace main_server{

class PackageDownloader {
public:
    PackageDownloader(folly::MPMCQueue<LightJSON> &download_queue,
        folly::MPMCQueue<HeavyJSON> &output_queue);
    void start();
    void stop();

private:
    void process_loop();
    void download_package(const LightJSON& package);
    static void store_to_database(const HeavyJSON& package);
    folly::MPMCQueue<LightJSON> &download_queue_;
    folly::MPMCQueue<HeavyJSON> &output_queue_;
    std::thread worker_;
    folly::CPUThreadPoolExecutor executor_;

    std::atomic<bool> is_running_{false};
};
}// namespace main_server


#endif //KUMYS_ARTIFACT_MANAGER_PACKAGEDOWNLOADER_H
