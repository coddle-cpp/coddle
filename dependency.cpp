#include "dependency.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "file_modification.hpp"
#include <iostream>

Dependency::Dependency(const std::string &fileName):
  fileName(fileName)
{
}

Dependency::~Dependency()
{
}

Dependency *Dependency::add(std::unique_ptr<Dependency> dependency)
{
  auto res = dependency.get();
  dependencyList.push_back(std::move(dependency));
  return res;
}

void Dependency::resolveTree()
{
  time_t maxTime = 0;
  for (auto &d: dependencyList)
    d->resolveTree();
  for (auto &d: dependencyList)
  {
    d->wait();
    maxTime = std::max(fileModification(d->fileName), maxTime);
  }
  if (isFileExist(fileName) && fileModification(fileName) >= maxTime)
    return;
  resolve();
}
