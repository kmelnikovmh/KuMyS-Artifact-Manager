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
namespace main_server {

class RequestHandler {
public:
  RequestHandler(folly::MPMCQueue<LightJSON> &input_queue,
                 folly::MPMCQueue<LightJSON> &download_queue,
                 folly::MPMCQueue<HeavyJSON> &output_queue,
                 folly::Executor::KeepAlive<> executor);

  void start();
  void stop();

private:
  folly::coro::Task<void> processLoop();
  folly::coro::Task<void> processPackage(LightJSON package);

  folly::MPMCQueue<LightJSON> &input_queue_;
  folly::MPMCQueue<LightJSON> &download_queue_;
  folly::MPMCQueue<HeavyJSON> &output_queue_;
  folly::Executor::KeepAlive<> executor_;

  std::vector<folly::SemiFuture<void>>  tasks_;
};

} // namespace main_server

#endif // KUMYS_ARTIFACT_MANAGER_REQUESTHANDLER_H
