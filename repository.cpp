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
  if (!isFileExist(repoDir))
    execShowCmd("git clone --depth 1", git, "-b", version, repoDir);

  load(repoDir);
}

void Repository::load(const std::string& repoDir)
{
  if (repoDir.empty())
    return;
  std::clog << "Loading config: " << repoDir << std::endl;
  if (!isFileExist(repoDir))
  {
    std::clog << "Warning: repository file does not exist: " << repoDir << std::endl;
    return;
  }
  // parse .toml file
  auto &&toml = cpptoml::parse_file(repoDir + "/libraries.toml");
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
