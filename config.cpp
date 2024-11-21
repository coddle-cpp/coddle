#include "config.hpp"
#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "file_name.hpp"
#include "osal.hpp"

Config::Config(int argc, char **argv)
  : target(fileName(getCurrentWorkingDir())),
    remoteRepository("https://github.com/coddle-cpp/coddle-repository.git"),
#ifdef _WIN32
    cflags("-Wall -Wextra -gdwarf-3 -D_GLIBCXX_DEBUG -I/usr/include -I/usr/local/include")
#elif __APPLE__
    cflags("-Wall -Wextra -gdwarf-3 -D_GLIBCXX_DEBUG -I/usr/include -I/usr/local/include")
#else
    cflags("-Wall -Wextra -gdwarf-3")
#endif
{
  for (int i = 1; i < argc; ++i)
    if (argv[i] == std::string("debug"))
      debug = true;
    else if (argv[i] == std::string("verbose"))
      verbose = true;
    else if (argv[i] == std::string("shared"))
      shared = true;
    else if (argv[i] == std::string("em") || argv[i] == std::string("emscripten"))
      emscripten = true; // New flag for Emscripten

  loadConfig("/etc/coddle.toml");
  loadConfig("~/.coddle.toml");
  loadConfig("coddle.toml");

  if (!emscripten)
  {
#if defined(_WIN32)
    loadConfig("coddle-win.toml");
#elif defined(__APPLE__)
    loadConfig("coddle-macos.toml");
#elif defined(__linux__)
    loadConfig("coddle-linux.toml");
#endif
  }
  else
    loadConfig("coddle-emscripten.toml");

  if (!emscripten)
  {
#ifdef _WIN32
    remoteVersion = "win";
#elif __APPLE__
    remoteVersion = "macosx";
#else
    remoteVersion = "master";
#endif
  }
  else
  {
    remoteVersion = "emscripten";

    cc = "emcc";
    cxx = "em++";
    ldflags += " -s WASM=1";
  }
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
    srcDir = toml->get_as<std::string>("srcDir").value_or(srcDir);
    targetDir = toml->get_as<std::string>("targetDir").value_or(targetDir);
    cflags = toml->get_as<std::string>("cflags").value_or(cflags);
    ldflags = toml->get_as<std::string>("ldflags").value_or(ldflags);
    debug = toml->get_as<bool>("debug").value_or(debug);
    multithreaded = toml->get_as<bool>("multithreaded").value_or(multithreaded);
    winmain = toml->get_as<bool>("winmain").value_or(winmain);
    shared = toml->get_as<bool>("shared").value_or(shared);
    cc = toml->get_as<std::string>("cc").value_or(cc);
    cxx = toml->get_as<std::string>("cxx").value_or(cxx);
    marchNative = toml->get_as<bool>("marchNative").value_or(marchNative);
    emscripten = toml->get_as<bool>("emscripten").value_or(emscripten); // Load from config
  }
}
