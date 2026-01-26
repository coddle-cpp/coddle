#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class ThreadPool
{
public:
  ThreadPool();
  ~ThreadPool();
  void addJob(std::function<void()> &&job, std::function<void()> &&afterJob);
  void waitForOne();
  bool empty();
  void join();

private:
  std::vector<std::pair<std::function<void()>, std::function<void()>>> jobs;
  std::vector<std::function<void()>> afterJobs;
  int runningJobs = 0;
  std::vector<std::thread> pool;
  bool done = false;
  std::mutex mutex;
  std::condition_variable newJob;
  std::condition_variable jobDone;
  void run();
};
