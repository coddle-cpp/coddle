#include "repository.hpp"

#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "osal.hpp"
#include <iostream>

Repository::Repository(const std::string &localRepoDir,
                       const std::string &git,
                       const std::string &version)
{
  if (!localRepoDir.empty())
  {
    local = localRepoDir + "/libraries.toml";
    load(local->name);
  }

  if (!git.empty() && !version.empty())
  {
    // clone git repository
    std::string repoDir = ".coddle/remote";
    if (!isFileExist(repoDir))
      execShowCmd("git clone --depth 1", git, "-b", version, repoDir);
    remote = repoDir + "/libraries.toml";
    load(remote->name);
  }

  for (auto &&lib : libraries)
    for (auto &&inc : lib.second.includes)
      incToLib[inc].push_back(&lib.second);
}

void Repository::load(const std::string& repo)
{
  std::clog << "Loading config: " << repo << std::endl;
  if (!isFileExist(repo))
  {
    std::clog << "Warning: repository file does not exist: " << repo << std::endl;
    return;
  }
  // parse .toml file
  auto &&toml = cpptoml::parse_file(repo);
  auto &&libs = toml->get_table_array("library");
  if (!libs)
    return;
  for (auto &&library : *libs)
  {
    auto &&name = library->get_as<std::string>("name");
    if (!name)
    {
      std::clog << "Warning: name is missing\n";
      continue;
    }
    auto &&lib = libraries[*name];
    lib.name = *name;
    auto &&typeStr = library->get_as<std::string>("type");
    if (!typeStr)
      throw std::runtime_error("Library type has to be specified: file, git, pkgconfig or lib");
    lib.type = toLibraryType(*typeStr);
    auto &&path = library->get_as<std::string>("path");
    if (!path && (lib.type == Library::Type::File || lib.type == Library::Type::Git))
    {
      std::clog << "Warning: path is missing\n";
      continue;
    }
    lib.path = *path;
    lib.postClone = library->get_as<std::string>("postClone").value_or("");
    lib.version = library->get_as<std::string>("version").value_or("master");
    lib.incdir = library->get_as<std::string>("incdir").value_or("");
    auto &&incdirs = library->get_array_of<std::string>("incdirs");
    if (incdirs)
      lib.incdirs = *incdirs;
    lib.libdir = library->get_as<std::string>("libdir").value_or("");

    auto &&includes = library->get_array_of<std::string>("includes");
    if (!includes)
    {
      std::clog << "Warning: includes are missing\n";
      continue;
    }
    lib.includes = *includes;

    auto &&dependencies = library->get_array_of<std::string>("dependencies");
    if (dependencies)
      lib.dependencies = *dependencies;
  }
}
