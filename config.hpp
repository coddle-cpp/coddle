#pragma once
#include <vector>
#include <string>

class Config
{
public:
  Config(int argc, char **argv);
  std::vector<std::string> cflags;
  std::vector<std::string> ldflags;
  std::vector<std::string> pkgs;
  std::string target;
  bool configured() const;
private:
  std::vector<std::string> args;
};
