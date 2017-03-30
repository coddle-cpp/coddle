#include "gcc_driver.hpp"
#include "config.hpp"
#include "file_name.hpp"
#include "gcc_binary.hpp"
#include "gcc_object.hpp"
#include "osal.hpp"

std::unique_ptr<Resolver> GccDriver::makeBinaryResolver(Config *config)
{
  auto target = config->target;
  if (target.empty())
  {
    target = fileName(getCurrentWorkingDir());
    if (config->targetType == TargetType::SharedLib)
      target = "lib" + target + ".so";
    else if (config->targetType == TargetType::StaticLib)
      target = "lib" + target + ".a";
  }
  return std::make_unique<Gcc::Binary>(target, config);
}

std::unique_ptr<Resolver> GccDriver::makeObjectResolver(const std::string &srcFile, Config *config)
{
  return std::make_unique<Gcc::Object>(srcFile, config);
}
