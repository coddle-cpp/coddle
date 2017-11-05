#include "resolver.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "osal.hpp"
#include <algorithm>
#include <iostream>

Resolver::Resolver(const std::string &fileName, Config *config, ProjectConfig *project)
  : fileName(fileName), config(config), project(project)
{
}

Resolver::~Resolver()
{
}

Resolver *Resolver::add(std::unique_ptr<Resolver> resolver)
{
  auto res = resolver.get();
  resolverList.push_back(std::move(resolver));
  return res;
}

void Resolver::resolveTree()
{
  if (runResolve)
    return;
  for (auto &d : resolverList)
    d->resolveTree();
  time_t maxTime = 0;
  for (auto &d : resolverList)
  {
    if (d->runResolve)
      d->wait();
    maxTime = std::max(getFileModification(d->fileName), maxTime);
  }
  preResolve();
  if (isFileExist(fileName) && getFileModification(fileName) >= maxTime)
    return;
  runResolve = true;
  resolve();
}

void Resolver::wait()
{
}

bool Resolver::isRunResolve() const
{
  return runResolve;
}

void Resolver::preResolve()
{
}
