#pragma once
#include <string>

class Config
{
public:
  Config(int argc, char **argv);
  std::string target;
  std::string remoteRepository;
  std::string remoteVersion;
  std::string localRepository;
private:
  void loadConfig(const std::string& configFileName);
};

