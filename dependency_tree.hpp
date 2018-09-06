#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <tuple>

class Resolver;
class DependencyTree
{
public:
  Resolver *addTarget(const std::string &fileName);
  void resolve();

private:
  std::unordered_map<std::string, std::unique_ptr<Resolver>> tree;
  std::unordered_set<std::string> resolvingList;
  std::unordered_set<std::string> resolvedList;
  using Resolvers = std::set<std::tuple<std::string, Resolver *, time_t>>;
  bool resolve(const std::string &target, Resolvers &resolvers);
};
