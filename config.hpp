#pragma once
#include "driver.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Resolver;

enum class TargetType { Unknown, Executable, StaticLib, SharedLib };

class Config
{
public:
  Config(int argc, char **argv);
  bool configured() const;
  std::string execPath() const;
  void configureForConfig();

  std::unique_ptr<Driver> driver;
  int njobs = 4;
  std::string target;
  TargetType targetType = TargetType::Unknown;
  std::unordered_map<std::string, std::vector<std::string> > incToLib;
  std::unordered_map<std::string, std::vector<std::string> > incToPkg;
  std::vector<std::string> args;
  std::vector<std::string> cflags;
  std::vector<std::string> ldflags;
  std::vector<std::string> libs;
  std::vector<std::string> pkgs;
  bool multithread = false;
};
