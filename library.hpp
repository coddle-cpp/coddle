#pragma once
#include <string>
#include <vector>

class Library
{
public:
  Library(const std::string &name = "",
          const std::string &git = "",
          const std::string &version = "");
  std::string name;
  std::string git;
  std::string version;
  std::vector<std::string> includes;
};
