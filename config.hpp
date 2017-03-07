#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Config
{
public:
  Config(int argc, char **argv);
  std::vector<std::string> cflags;
  std::vector<std::string> ldflags;
  std::vector<std::string> pkgs;
  std::unordered_map<std::string, std::vector<std::string> > incToPkg;
  std::string target;
  int njobs = 4;
  bool configured() const;
  std::vector<std::string> args;
  std::string exe() const;
};