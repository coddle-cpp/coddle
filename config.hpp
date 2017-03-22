#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Dependency;
class Config
{
public:
  Config(int argc, char **argv);
  bool configured() const;
  int njobs = 4;
  std::function<std::unique_ptr<Dependency> (Config *)> binaryFactory;
  std::function<std::unique_ptr<Dependency> (const std::string &srcFileName, Config *)> objectFactory;
  std::function<void (Dependency *, const std::string &srcFileName)> addDependency;
  std::string execPath() const;
  std::string target;
  std::unordered_map<std::string, std::vector<std::string> > incToLib;
  std::unordered_map<std::string, std::vector<std::string> > incToPkg;
  std::vector<std::string> args;
  std::vector<std::string> cflags;
  std::vector<std::string> ldflags;
  std::vector<std::string> libs;
  std::vector<std::string> pkgs;
  void configureForConfig();
};
