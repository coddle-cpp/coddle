#include "config.hpp"

#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "file_name.hpp"
#include "osal.hpp"

Config::Config(int argc, char **argv)
  : target(fileName(getCurrentWorkingDir())),
    remoteRepository("https://github.com/antonte/coddle-repository.git"),
#ifdef _WIN32
    remoteVersion("win"),
#elif __APPLE__
    remoteVersion("macosx"),
#else
    remoteVersion("master"),
#endif
    cflags("-Wall -Wextra -march=native -gdwarf-3 -D_GLIBCXX_DEBUG -std=c++17"),
    debug(false)
{
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string("debug"))
    {
      debug = true;
      break;
    }

  loadConfig("/etc/coddle.toml");
  loadConfig("~/.coddle.toml");
  loadConfig("coddle.toml");
}

void Config::loadConfig(const std::string &configFileName)
{
  if (isFileExist(configFileName))
  {
    auto &&toml = cpptoml::parse_file(configFileName);
    target = toml->get_as<std::string>("target").value_or(target);
    remoteRepository = toml->get_as<std::string>("remoteRepository").value_or(remoteRepository);
    remoteVersion = toml->get_as<std::string>("remoteVersion").value_or(remoteVersion);
    localRepository = toml->get_as<std::string>("localRepository").value_or(localRepository);
    cflags = toml->get_as<std::string>("cflags").value_or(cflags);
    debug = toml->get_as<bool>("debug").value_or(debug);
    multithreaded = toml->get_as<bool>("multithreaded").value_or(false);
  }
}
