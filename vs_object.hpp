#pragma once

#include "resolver.hpp"
#include <condition_variable>
#include <mutex>

class Config;
namespace Vs
{
class Object: public Resolver
{
public:
  Object(const std::string &source, Config *);
  bool hasMain() const;
private:
  void resolve() override;
  void wait() override;
  std::string source;
  bool resolved = true;
  std::mutex mutex;
  std::condition_variable cond;
  std::string errorString;
  void job();
};
}
