#pragma once
#include <string>

class Config
{
public:
  Config(int argc = 0, char **argv = nullptr);
  std::string target;
  std::string remoteRepository;
  std::string remoteVersion;
  std::string localRepository;
  std::string srcDir;
  std::string targetDir;
  std::string artifactsDir;
  std::string cflags;
  bool debug{false};
  bool multithreaded{false};
  bool winmain{false};
  bool verbose{false};
  bool shared{false};

private:
  void loadConfig(const std::string &configFileName);
};
