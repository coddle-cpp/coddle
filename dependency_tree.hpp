#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Resolver;
class DependencyTree
{
public:
  Resolver *addTarget(const std::string &fileName);
  void resolve();

private:
  std::unordered_map<std::string, std::unique_ptr<Resolver>> tree;
  std::unordered_set<std::string> resolvedList;
  void resolve(const std::string &target);
};
