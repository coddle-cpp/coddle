#include "exec_pool.hpp"
#include "config.hpp"
#include <iostream>

ExecPool &ExecPool::instance(Config *config)
{
  static ExecPool pool(config);
  return pool;
}

ExecPool::ExecPool(Config *cfg) : config(cfg)
{
  for (int i = 0; i < config->njobs; ++i)
    workersList.emplace_back(std::bind(&ExecPool::workerLoop, this));
}

void ExecPool::finalize()
{
  if (!done)
  {
    {
      std::lock_guard<std::mutex> l(mutex);
      done = true;
      cond.notify_all();
    }
    for (auto &worker : workersList)
      worker.join();
  }
}

ExecPool::~ExecPool()
{
  finalize();
}

void ExecPool::submit(std::function<void()> job)
{
  std::lock_guard<std::mutex> l(mutex);
  jobQueue.push_back(job);
  cond.notify_all();
}

void ExecPool::workerLoop()
{
  mutex.lock();
  while (!done)
  {
    if (!jobQueue.empty())
    {
      auto job = jobQueue.front();
      jobQueue.pop_front();
      mutex.unlock();
      try
      {
        job();
      }
      catch (...)
      {
        std::cerr << "Uncaught exception in the worker thread\n";
      }
    }
    else
      mutex.unlock();
    {
      std::unique_lock<std::mutex> l(mutex);
      if (!done && jobQueue.empty())
      {
        cond.wait(l);
      }
    }
    mutex.lock();
  }
  mutex.unlock();
}
