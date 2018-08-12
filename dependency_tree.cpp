#include "dependency_tree.hpp"

#include "osal.hpp"
#include "file_exist.hpp"
#include "resolver.hpp"
#include <algorithm>

Resolver *DependencyTree::addTarget(const std::string &fileName)
{
  auto &&res = tree.insert(std::make_pair(fileName, std::make_unique<Resolver>()));
  if (!res.second)
    throw std::runtime_error("Duplicated target: " + fileName);
  return res.first->second.get();
}

void DependencyTree::resolve()
{
  resolvedList.clear();
  for (auto &&target : tree)
    resolve(target.first);
}

// TODO run in parallel
void DependencyTree::resolve(const std::string &target)
{
  auto &&resolver = tree.find(target);
  if (resolver == std::end(tree))
  {
    resolvedList.insert(target);
    return;
  }
  time_t newestModificationTime = 0;
  for (auto &&dependency : resolver->second->dependencies)
  {
    resolve(dependency);
    newestModificationTime = std::max(getFileModification(dependency), newestModificationTime);
  }
  if (!isFileExist(target))
  {
    resolver->second->exec();
    resolvedList.insert(target);
    return;
  }
  auto time = getFileModification(target);
  if (time < newestModificationTime)
    resolver->second->exec();
  resolvedList.insert(target);
  return;
}
