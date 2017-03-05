#pragma once
#include <string>
#include <memory>
#include <vector>

class Config;
class Dependency
{
public:
  Dependency(const std::string &fileName, Config *);
  virtual ~Dependency();
  std::string fileName;
  Dependency *add(std::unique_ptr<Dependency>);
  void resolveTree();
  bool isRunResolve() const;
protected:
  virtual void resolve() = 0;
  virtual void wait();
  std::vector<std::unique_ptr<Dependency>> dependencyList;
  Config *config;
  bool runResolve = false;
};
