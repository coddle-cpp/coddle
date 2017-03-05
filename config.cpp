#include "config.hpp"
#include <iostream>

Config::Config(int argc, char **argv)
{
  for (auto i = 0; i < argc; ++i)
    args.push_back(argv[0]);
  cflags.push_back("-Wall");
  cflags.push_back("-Wextra");
  cflags.push_back("-march=native");
  cflags.push_back("-gdwarf-3");
  cflags.push_back("-std=c++1y");
  cflags.push_back("-O3");
  cflags.push_back("-g");
}

bool Config::configured() const
{
  return args[0].find("coddle.cfg") != std::string::npos;
}
