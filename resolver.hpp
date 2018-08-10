#pragma once
#include <functional>
#include <string>
#include <unordered_set>

class Resolver
{
public:
  void dependsOf(const std::string &fileName);
  std::function<void()> exec;
  std::unordered_set<std::string> dependencies;
};
