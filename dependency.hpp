#pragma once
#include <string>
#include <memory>
#include <vector>

class Dependency
{
public:
  Dependency(const std::string &fileName);
  virtual ~Dependency();
  std::string fileName;
  Dependency *add(std::unique_ptr<Dependency>);
  void resolveTree();
protected:
  virtual void resolve() = 0;
  virtual void wait() = 0;
  std::vector<std::unique_ptr<Dependency>> dependencyList;
};
