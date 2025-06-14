//
// Created by Kymus-team on 2/22/25.
//
#ifndef KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
#define KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H

#include "HeavyJson.h"
#include "LightJson.h"
#include <folly/MPMCQueue.h>
#include <folly/experimental/coro/Task.h>
#include <folly/futures/Future.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

namespace main_server {

    class RequestHandler {
    public:
        RequestHandler(folly::MPMCQueue<LightJSON> &input_queue,
                       folly::MPMCQueue<LightJSON> &download_queue,
                       folly::MPMCQueue<HeavyJSON> &output_queue);

        void start();
        void stop();

    private:
        void processLoop();
        folly::coro::Task<void> processPackage(LightJSON package);

        folly::MPMCQueue<LightJSON> &input_queue_;
        folly::MPMCQueue<LightJSON> &download_queue_;
        folly::MPMCQueue<HeavyJSON> &output_queue_;
        std::thread worker_;
        std::shared_ptr<folly::CPUThreadPoolExecutor> executor_;


        std::atomic<bool> stopped_{false};
    };

} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
