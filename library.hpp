#pragma once
#include <string>
#include <vector>

class Library
{
public:
  enum class Type { File, Git, PkgConfig, Lib };
  Library(Type type = Type::Git,
          const std::string &name = "",
          const std::string &git = "",
          const std::string &version = "");
  Type type;
  std::string name;
  std::string path;
  std::string version; // only for git
  std::vector<std::string> includes;
};
