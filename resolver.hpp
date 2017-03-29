#pragma once
#include <string>
#include <memory>
#include <vector>

class Config;
class Resolver
{
public:
  Resolver(const std::string &fileName, Config *);
  virtual ~Resolver();
  std::string fileName;
  Resolver *add(std::unique_ptr<Resolver>);
  void resolveTree();
  bool isRunResolve() const;
protected:
  virtual void resolve() = 0;
  virtual void wait();
  std::vector<std::unique_ptr<Resolver>> resolverList;
  Config *config;
  bool runResolve = false;
};
