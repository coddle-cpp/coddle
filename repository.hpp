#pragma once
#include "library.hpp"
#include <unordered_map>

class Repository
{
public:
  void load(const std::string &name, const std::string &git, const std::string &version);
  std::unordered_map<std::string, Library> libraries;
};
