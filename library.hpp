#pragma once
#include <string>
#include <vector>

class Library
{
public:
  enum class Type { File, Git, PkgConfig, Lib };
  Type type;
  std::string name;
  std::string path;
  std::string version; // only for git
  std::string postClone;
  std::vector<std::string> includes;
  std::string incdir;
  std::string libdir;
};
