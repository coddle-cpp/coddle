#pragma once
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class Config;
class ExecPool
{
public:
  ExecPool(Config *);
  ~ExecPool();
  ExecPool(const ExecPool &) = delete;
  ExecPool &operator=(const ExecPool &) = delete;
  static ExecPool &instance(Config *);
  void submit(std::function<void()> job);
private:
  void workerLoop();
  Config *config;
  bool done = false;
  int jobs = 0;
  std::mutex mutex;
  std::condition_variable cond;
  std::deque<std::function<void()>> jobQueue;
  std::vector<std::thread> workersList;
};
