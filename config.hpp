#pragma once
#include "macro.hpp"
#include "ser.hpp"
#include <string>

class Config
{
public:
  Config(int argc = 0, char **argv = nullptr);
  std::string target;
  std::string remoteRepository;
  std::string remoteVersion;
  std::string localRepository;
  std::string srcDir{"."};
  std::string targetDir{"."};
  std::string artifactsDir{".coddle"};
  std::string cflags;
  std::string ldflags;
  bool debug{false};
  bool multithreaded{false};
  bool winmain{false};
  bool verbose{false};
  bool shared{false};
  bool fpic{false};
  std::string cc{"clang"};
  std::string cxx{"clang++"};
  bool marchNative{true};

#define SER_PROPERTY_LIST         \
  SER_PROPERTY(target);           \
  SER_PROPERTY(remoteRepository); \
  SER_PROPERTY(remoteVersion);    \
  SER_PROPERTY(localRepository);  \
  SER_PROPERTY(srcDir);           \
  SER_PROPERTY(targetDir);        \
  SER_PROPERTY(artifactsDir);     \
  SER_PROPERTY(cflags);           \
  SER_PROPERTY(ldflags);          \
  SER_PROPERTY(debug);            \
  SER_PROPERTY(multithreaded);    \
  SER_PROPERTY(winmain);          \
  SER_PROPERTY(verbose);          \
  SER_PROPERTY(shared);           \
  SER_PROPERTY(fpic);             \
  SER_PROPERTY(cc);               \
  SER_PROPERTY(cxx);              \
  SER_PROPERTY(marchNative);

  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST

private:
  void loadConfig(const std::string &configFileName);
};
