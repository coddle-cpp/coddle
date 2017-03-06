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
  bool configured() const;
private:
  std::vector<std::string> args;
};
