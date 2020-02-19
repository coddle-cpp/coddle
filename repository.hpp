#pragma once
#include "file.hpp"
#include "library.hpp"
#include "ser.hpp"
#include <optional>
#include <unordered_map>

class Repository
{
public:
  Repository(const std::string &localRepoDir, const std::string &git, const std::string &version);
  std::optional<File> local;
  std::optional<File> remote;

#define SER_PROPERTY_LIST \
  SER_PROPERTY(local);    \
  SER_PROPERTY(remote);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST

  std::unordered_map<std::string, Library> libraries;
  using IncToLib = std::unordered_map<std::string, std::vector<const Library *>>;
  IncToLib incToLib;

private:
  void load(const std::string &repo);
};
