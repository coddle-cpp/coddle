#include "thread_pool.hpp"
#include <exception>
#include <optional>

ThreadPool::ThreadPool()
{
  for (auto i = 0u; i < std::thread::hardware_concurrency(); ++i)
    pool.emplace_back(&ThreadPool::run, this);
}

ThreadPool::~ThreadPool()
{
  join();
}

void ThreadPool::join()
{
  {
    std::lock_guard<std::mutex> guard(mutex);
    done = true;
  }
  newJob.notify_all();

  auto tmpPool = std::move(pool);
  pool.clear();
  for (auto &&p : tmpPool)
    p.join();
  auto tmpAfterJobs = std::move(afterJobs);
  afterJobs.clear();
  for (auto &&job : tmpAfterJobs)
  {
    // Swallow exceptions during join to avoid terminate in destructors
    // when an exception is already propagating from waitForOne().
    try
    {
      job();
    }
    catch (...)
    {
    }
  }
}

void ThreadPool::addJob(std::function<void()> &&job, std::function<void()> &&afterJob)
{
  std::lock_guard<std::mutex> guard(mutex);
  jobs.push_back(std::make_pair(job, afterJob));
  newJob.notify_one();
}

void ThreadPool::waitForOne()
{
  std::unique_lock<std::mutex> lock(mutex);
  if (!afterJobs.empty())
  {
    for (auto &&job : afterJobs)
      job();
    afterJobs.clear();
    return;
  }
  if (jobs.empty() && runningJobs == 0)
    return;

  jobDone.wait(lock);

  for (auto &&job : afterJobs)
    job();
  afterJobs.clear();
}

bool ThreadPool::empty()
{
  return jobs.empty();
}

void ThreadPool::run()
{
  for (;;)
  {
    try
    {
      auto job = [this]() -> std::optional<std::pair<std::function<void()>, std::function<void()>>> {
        std::unique_lock<std::mutex> lock(mutex);
        while (jobs.empty())
        {
          if (done)
            return std::nullopt;
          newJob.wait(lock);
          if (done)
            return std::nullopt;
        }

        auto res = jobs.back();
        jobs.pop_back();
        ++runningJobs;
        return res;
      }();

      if (!job)
        return;

      job->first();
      {
        std::lock_guard<std::mutex> guard(mutex);
        afterJobs.push_back(job->second);
      }
      {
        std::unique_lock<std::mutex> lock(mutex);
        --runningJobs;
      }
      jobDone.notify_one();
    }
    catch (std::exception &e)
    {
      {
        std::lock_guard<std::mutex> guard(mutex);
        std::string err = e.what();
        afterJobs.push_back([err]() { throw std::runtime_error(err); });
      }
      jobDone.notify_one();
    }
    catch (int e)
    {
      {
        std::lock_guard<std::mutex> guard(mutex);
        afterJobs.push_back([e]() { throw e; });
      }
      jobDone.notify_one();
    }
  }
}
