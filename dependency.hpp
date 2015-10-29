#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

class Dependency
{
public:
  static Dependency *get(const std::string &);
  virtual void resolve() = 0;
  void add(const std::string &fileName);
private:
  static std::unordered_map<std::string, std::shared_ptr<Dependency> > dependencyDb_;
  std::vector<Dependency *> dependencyList_;
};
