#pragma once
#include "repository.hpp"
#include <string>
#include <unordered_set>

class Config;

class Coddle
{
public:
  int build(const Config &);
  Repository repository;

private:
  std::unordered_set<std::string> libs;
  std::unordered_set<std::string> pkgs;
  std::unordered_set<std::string> generateLibsFiles(const Config &) const;
  bool downloadAndBuildLibs(const Config &,
                                 const std::unordered_set<std::string> &localLibs);
  void generateProjectLibsFile(const Config &config) const;
};
