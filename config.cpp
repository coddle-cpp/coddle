#include "config.hpp"

#include "file_exist.hpp"
#include "cpptoml/cpptoml.h"

Config::Config(int /*argc*/, char **/*argv*/)
{
  loadConfig("/etc/coddle.toml");
  loadConfig("~/.coddle.toml");
  loadConfig("coddle.toml");
}

void Config::loadConfig(const std::string& configFileName)
{
  if (isFileExist(configFileName))
  {
    auto &&toml = cpptoml::parse_file(configFileName);
    remoteRepository = toml->get_as<std::string>("remoteRepository").value_or(remoteRepository);
    remoteVersion = toml->get_as<std::string>("remoteVersion").value_or(remoteVersion);
    localRepository = toml->get_as<std::string>("localRepository").value_or(localRepository);
    localVersion = toml->get_as<std::string>("localVersion").value_or(localVersion);
  }
}

