#include "dependency_tree.hpp"

#include "osal.hpp"
#include "file_exist.hpp"
#include "resolver.hpp"
#include "thread_pool.hpp"
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
  resolvingList.clear();
  ThreadPool threadPool;
  for (;;)
  {
    Resolvers resolvers;
    for (auto &&target : tree)
      resolve(target.first, resolvers);

    if (resolvers.empty() && resolvingList.empty() && threadPool.empty())
      return;

    std::string target;
    Resolver *resolver;
    time_t newestModificationTime;

    for (auto &&r : resolvers)
    {
      std::tie(target, resolver, newestModificationTime) = r;
      if (!isFileExist(target))
      {
        resolvingList.insert(target);
        threadPool.addJob([resolver]() { resolver->exec(); },
                          [this, target]() {
                            resolvingList.erase(target);
                            resolvedList.insert(target);
                          });
        continue;
      }
      auto time = getFileModification(target);
      if (time < newestModificationTime)
      {
        resolvingList.insert(target);
        threadPool.addJob([resolver]() { resolver->exec(); },
                          [this, target]() {
                            resolvingList.erase(target);
                            resolvedList.insert(target);
                          });
      }
      else
        resolvedList.insert(target);
    }
    threadPool.waitForOne();
  }
}

bool DependencyTree::resolve(const std::string &target,
                             Resolvers &resolvers)
{
  if (resolvedList.find(target) != std::end(resolvedList))
    return true;
  if (resolvingList.find(target) != std::end(resolvingList))
    return false;
  auto &&resolver = tree.find(target);
  if (resolver == std::end(tree))
  {
    resolvedList.insert(target);
    return true;
  }

  Resolvers tmpResolvers;
  for (auto &&dependency : resolver->second->dependencies)
    if (!resolve(dependency, tmpResolvers))
      return false;
  if (!tmpResolvers.empty())
  {
    resolvers.insert(std::begin(tmpResolvers), std::end(tmpResolvers));
    return true;
  }
  time_t newestModificationTime = 0;
  for (auto &&dependency : resolver->second->dependencies)
    newestModificationTime = std::max(getFileModification(dependency), newestModificationTime);
  resolvers.insert(std::make_tuple(target, resolver->second.get(), newestModificationTime));
  return true;
}
