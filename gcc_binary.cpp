#include "gcc_binary.hpp"
#include "config.hpp"
#include "error.hpp"
#include "gcc_object.hpp"
#include "osal.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Gcc
{
void Binary::resolve()
{
  if (resolverList.empty())
  {
    std::cerr << "Nothing to build\n";
    return;
  }

  if (project->targetType == TargetType::Unknown)
  {
    for (auto &resolver: resolverList)
    {
      std::ifstream nmFile(resolver->fileName + ".nm");
      std::string line;
      while (std::getline(nmFile, line))
        if (line.find(" T main") != std::string::npos ||
            line.find(" T _main") != std::string::npos)
        {
          project->targetType = TargetType::Executable;
        }
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
