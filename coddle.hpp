#pragma once
#include "repository.hpp"
#include <string>
#include <unordered_set>

class Config;

class Coddle
{
public:
  int exec(const Config &);
  Repository repository;

private:
  std::unordered_set<std::string> libs;
  std::unordered_set<std::string> pkgs;
};
