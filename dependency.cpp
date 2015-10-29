#include "dependency.hpp"
#include "binary.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

std::unordered_map<std::string, std::shared_ptr<Dependency> > Dependency::dependencyDb_;


bool isDirectory(const std::string &path)
{
  struct stat s;
  if (stat(path.c_str(), &s) == 0)
  {
    if( s.st_mode & S_IFDIR )
      return true;
  }
  else
    throw std::runtime_error("Stat error: '" + path + "'");
  return false;
}

Dependency *Dependency::get(const std::string &path)
{
  auto iter = dependencyDb_.find(path);
  if (iter == std::end(dependencyDb_))
  {
    std::shared_ptr<Dependency> ptr;
    if (isDirectory(path))
      ptr = std::make_shared<Binary>(path);
    else
    {
      // TODO
    }
    iter = dependencyDb_.insert(std::make_pair(path, ptr)).first;
  }
  return iter->second.get();
}

void Dependency::add(const std::string &fileName)
{
  dependencyList_.push_back(get(fileName));
}
