#include "config.hpp"

#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "file_name.hpp"
#include "osal.hpp"

Config::Config(int /*argc*/, char ** /*argv*/)
{
  loadConfig("/etc/coddle.toml");
  loadConfig("~/.coddle.toml");
  loadConfig("coddle.toml");
}

void Config::loadConfig(const std::string &configFileName)
{
  if (isFileExist(configFileName))
  {
    auto &&toml = cpptoml::parse_file(configFileName);
    target = toml->get_as<std::string>("target").value_or(fileName(getCurrentWorkingDir()));
    remoteRepository = toml->get_as<std::string>("remoteRepository").value_or(remoteRepository);
    remoteVersion = toml->get_as<std::string>("remoteVersion").value_or(remoteVersion);
    localRepository = toml->get_as<std::string>("localRepository").value_or(localRepository);
  }
}
