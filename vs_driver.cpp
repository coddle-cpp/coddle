#include "vs_driver.hpp"
#include "config.hpp"
#include "file_name.hpp"
#include "osal.hpp"
#include "vs_binary.hpp"
#include "vs_object.hpp"

std::unique_ptr<Resolver> VsDriver::makeBinaryResolver(Config *config)
{
  auto target = config->target;
  if (target.empty())
  {
    target = fileName(getCurrentWorkingDir());
    target += ".exe";
  }
  return std::make_unique<Vs::Binary>(target, config);
}

std::unique_ptr<Resolver> VsDriver::makeObjectResolver(const std::string &srcFile, Config *config)
{
  return std::make_unique<Vs::Object>(srcFile, config);
}
