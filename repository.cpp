#include "repository.hpp"

#include "cpptoml/cpptoml.h"
#include "file_exist.hpp"
#include "osal.hpp"
#include <iostream>

void Repository::load(const std::string &name, const std::string &git, const std::string &version)
{
  if (git.empty() || version.empty())
    return;

  // clone git repository
  auto &&repoDir = ".coddle/" + name;
  if (!isFileExist(repoDir))
  {
    makeDir(".coddle");
    execShowCmd("git clone --depth 1", git, "-b", version, repoDir);
  }
  else
    execShowCmd("cd " + repoDir + "&& git pull");

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
    auto &&git = library->get_as<std::string>("git");
    if (!git)
    {
      std::clog << "Warning: git is missing\n";
      continue;
    }
    auto &&version = library->get_as<std::string>("version").value_or("master");
    auto &&lib = libraries[*name];
    lib = Library(*name, *git, version);
    
    auto &&includes = library->get_array_of<std::string>("includes");
    if (!includes)
    {
      std::clog << "Warning: includes are missing\n";
      continue;
    }
    lib.includes = *includes;
  }
}
