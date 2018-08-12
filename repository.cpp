#include "repository.hpp"

#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "osal.hpp"
#include <iostream>

void Repository::load(const std::string &git, const std::string &version)
{
  if (git.empty() || version.empty())
    return;

  // clone git repository
  std::string repoDir = ".coddle/remote";
  // TODO handle version change
  if (!isFileExist(repoDir))
    execShowCmd("git clone --depth 1", git, "-b", version, repoDir);
  else
    execShowCmd("cd " + repoDir + "&& git pull");

  load(repoDir);
}

void Repository::load(const std::string& repoDir)
{
  // parse .toml file
  auto &&toml = cpptoml::parse_file(repoDir + "/libraries.toml");
  auto &&libs = toml->get_table_array("library");
  if (!libs)
    return;
  for (auto &&library : *libs)
  {
    auto &&typeStr = library->get_as<std::string>("type");
    auto type = [&]() {
      if (!typeStr)
        throw std::runtime_error("Library type has to be specified: file, git, pkgconfig or lib");
      if (*typeStr == "file")
        return Library::Type::File;
      else if (*typeStr == "git")
        return Library::Type::Git;
      else if (*typeStr == "pkgconfig")
        return Library::Type::PkgConfig;
      else if (*typeStr == "lib")
        return Library::Type::Lib;
      else
        throw std::runtime_error("Unknwon library type: " + *typeStr);
    }();
    auto &&name = library->get_as<std::string>("name");
    if (!name)
    {
      std::clog << "Warning: name is missing\n";
      continue;
    }
    auto &&path = library->get_as<std::string>("path");
    if (!path && (type == Library::Type::File || type == Library::Type::Git))
    {
      std::clog << "Warning: path is missing\n";
      continue;
    }
    auto &&version = library->get_as<std::string>("version").value_or("master");
    auto &&lib = libraries[*name];
    lib = Library(type, *name, *path, version);

    auto &&includes = library->get_array_of<std::string>("includes");
    if (!includes)
    {
      std::clog << "Warning: includes are missing\n";
      continue;
    }
    lib.includes = *includes;
  }
}
