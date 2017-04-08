#include "gcc_binary.hpp"
#include "config.hpp"
#include "error.hpp"
#include "gcc_object.hpp"
#include "osal.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

static void resolveLibs(const ProjectConfig *project, const Config *config, std::vector<std::string> &internalLibs)
{
  for (const auto &p: config->projects)
  {
    if (&p == project)
      break;
    if (p.targetType != TargetType::SharedLib && p.targetType != TargetType::StaticLib)
      continue;
    for (const auto &sym: project->neededSymbols)
    {
      if (p.providedSymbols.find(sym) != std::end(p.providedSymbols))
      {
        if (std::find(std::begin(internalLibs), std::end(internalLibs), p.target) == std::end(internalLibs))
        {
          internalLibs.push_back(p.target);
          if (p.targetType == TargetType::StaticLib)
            resolveLibs(&p, config, internalLibs);
        }
        break;
      }
    }
  }
}

namespace Gcc
{
void Binary::resolve()
{
  if (resolverList.empty())
  {
    std::cerr << "Nothing to build\n";
    return;
  }

  project->target = fileName;
  project->neededSymbols.clear();
  project->providedSymbols.clear();
  for (auto &resolver: resolverList)
  {
    std::ifstream nmFile(resolver->fileName + ".nm");
    std::string line;
    while (std::getline(nmFile, line))
    {
      if (project->targetType == TargetType::Unknown)
      {
        if (line.find(" T main") != std::string::npos ||
            line.find(" T _main") != std::string::npos)
          project->targetType = TargetType::Executable;
      }
      auto p = line.rfind(" ");
      if (p == std::string::npos || p < 1)
        continue;
      auto sym = line.substr(p + 1);
      auto type = line[p - 1];
      if (type == 'T')
        project->providedSymbols.insert(sym);
      else
        project->neededSymbols.insert(sym);
    }
  }

  std::string objList;
  for (auto &resolver: resolverList)
    if (dynamic_cast<Gcc::Object *>(resolver.get()))
      objList += resolver->fileName + " ";
  try
  {
    std::ostringstream strm;
    if (project->targetType == TargetType::Executable || project->targetType == TargetType::SharedLib)
    {
      strm << "g++";
      if (project->targetType == TargetType::SharedLib)
        strm << " -shared";
      strm << " " << objList << "-o " << fileName;
      auto ldflags = Config::merge(config->common.ldflags, project->ldflags);
      for (const auto &flag: ldflags)
        strm << " " << flag;
      if (config->multithread && std::find(std::begin(ldflags), std::end(ldflags), "-pthread") == std::end(ldflags))
        strm << " -pthread";
      auto libs = Config::merge(config->common.libs, project->libs);
      for (const auto &lib: libs)
        strm << " -l" << lib;
      {
        std::vector<std::string> internalLibs;
        resolveLibs(project, config, internalLibs);
        if (!internalLibs.empty())
          strm << " -L.";
        for (const auto &lib: internalLibs)
          strm << " -l:" << lib;
      }
      auto pkgs = Config::merge(config->common.pkgs, project->pkgs);
      if (!pkgs.empty())
      {
        strm << " $(pkg-config --libs";
        for (const auto &pkg: pkgs)
          strm << " " << pkg;
        strm << ")";
      }
    }
    else if (project->targetType == TargetType::StaticLib)
    {
      strm <<
        "ar rv " << fileName << " " << objList;
    }
    else
    {
      THROW_ERROR("Unknown binary type");
    }
    std::cout << strm.str() << std::endl;
    exec(strm.str());
  }
  catch (std::exception &e)
  {
    THROW_ERROR("coddle: *** [" << fileName << "] " << e.what())
  }
}
}
