#include "vs_driver.hpp"
#include "config.hpp"
#include "file_name.hpp"
#include "osal.hpp"
#include "vs_binary.hpp"
#include "vs_object.hpp"

std::unique_ptr<Resolver> VsDriver::makeBinaryResolver(Config *config, ProjectConfig *project)
{
  auto target = project->target;
  if (target.empty())
  {
    target = fileName(getCurrentWorkingDir());
    if (project->targetType == TargetType::SharedLib)
      target += ".dll";
    else if (project->targetType == TargetType::StaticLib)
      target += ".lib";
    else
      target += ".exe";
  }
  return std::make_unique<Vs::Binary>(target, config, project);
}

std::unique_ptr<Resolver> VsDriver::makeObjectResolver(const std::string &srcFile,
                                                       Config *config,
                                                       ProjectConfig *project)
{
  return std::make_unique<Vs::Object>(srcFile, config, project);
}
