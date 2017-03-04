#include "exec_pool.hpp"
#include <iostream>

ExecPool &ExecPool::instance()
{
  static ExecPool pool;
  return pool;
}

ExecPool::ExecPool():
  thread(std::bind(&ExecPool::exec, this))
{
}

ExecPool::~ExecPool()
{
  {
    std::lock_guard<std::mutex> l(mutex);
    done = true;
    cond.notify_all();
  }
  thread.join();
}

void ExecPool::submit(std::function<void()> job)
{
  std::lock_guard<std::mutex> l(mutex);
  jobQueue.push_back(job);
  cond.notify_all();
}

void ExecPool::exec()
{
  std::unique_lock<std::mutex> l(mutex);
  while (!done)
  {
    if (!jobQueue.empty() && jobs < 4)
    {
      auto job = jobQueue.front();
      jobQueue.pop_front();
      ++jobs;
      std::thread t([this, job]()
                    {
                      job();
                      {
                        std::lock_guard<std::mutex> l(mutex);
                        --jobs;
                      }
                      cond.notify_all();
                    });
      t.detach();
    }
    cond.wait(l);
  }
}
