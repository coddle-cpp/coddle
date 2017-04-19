#pragma once
#include <string>
#include <memory>
#include <vector>

class Config;
struct ProjectConfig;
class Resolver
{
public:
  Resolver(const std::string &fileName, Config *, ProjectConfig *);
  virtual ~Resolver();
  std::string fileName;
  Resolver *add(std::unique_ptr<Resolver>);
  void resolveTree();
  bool isRunResolve() const;
protected:
  virtual void preResolve();
  virtual void resolve() = 0;
  virtual void wait();
  std::vector<std::unique_ptr<Resolver>> resolverList;
  Config *config;
  ProjectConfig *project;
  bool runResolve = false;
};
