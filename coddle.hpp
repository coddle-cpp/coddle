#pragma once
#include "repository.hpp"
#include <fstream>
#include <string>
#include <unordered_set>

class Config;

class Coddle
{
public:
  Coddle();
  bool build(const Config &);
  Repository repository;

private:
  std::vector<std::pair<std::string, bool>> globalLibs;
  std::vector<std::pair<std::string, bool>>::const_iterator globalLibsFind(
    const std::string &) const;
  std::vector<std::pair<std::string, bool>>::iterator globalLibsFind(const std::string &);
  void globalLibsPushBack(const std::pair<std::string, bool> &);
  std::unordered_set<std::string> pkgs;
  std::unordered_set<std::string> generateLibsFiles(const Config &) const;
  bool downloadAndBuildLibs(const Config &, const std::unordered_set<std::string> &localLibs);
  void generateProjectLibsFile(const Config &config) const;
  std::ostream &debug() const;
  bool verbose{false};
  mutable std::ofstream nullstrm;
};
