#include "gcc_driver.hpp"
#include "config.hpp"
#include "file_name.hpp"
#include "gcc_binary.hpp"
#include "gcc_object.hpp"
#include "make_path.hpp"
#include "osal.hpp"

std::unique_ptr<Resolver> GccDriver::makeBinaryResolver(Config *config, ProjectConfig *project)
{
  auto target = project->target;
  if (target.empty())
  {
    if (project->srcDirs[0] != ".")
      target = fileName(makePath(getCurrentWorkingDir(), project->srcDirs[0]));
    else
      target = fileName(getCurrentWorkingDir());
    if (project->targetType == TargetType::SharedLib)
      target = "lib" + target + ".so";
    else if (project->targetType == TargetType::StaticLib)
      target = "lib" + target + ".a";
    target = makePath(project->srcDirs[0], target);
  }
  return std::make_unique<Gcc::Binary>(target, config, project);
}

std::unique_ptr<Resolver> GccDriver::makeObjectResolver(const std::string &srcFile, Config *config, ProjectConfig *project)
{
  return std::make_unique<Gcc::Object>(srcFile, config, project);
}
