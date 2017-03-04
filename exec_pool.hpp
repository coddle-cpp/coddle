#pragma once
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

class ExecPool
{
public:
  ExecPool();
  ~ExecPool();
  ExecPool(const ExecPool &) = delete;
  ExecPool &operator=(const ExecPool &) = delete;
  static ExecPool &instance();
  void submit(std::function<void()> job);
private:
  void exec();
  bool done = false;
  int jobs = 0;
  std::mutex mutex;
  std::condition_variable cond;
  std::deque<std::function<void()>> jobQueue;
  std::thread thread;
};
