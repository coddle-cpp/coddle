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

  if (config->targetType == TargetType::Unknown)
  {
    for (auto &resolver: resolverList)
    {
      std::ifstream nmFile(resolver->fileName + ".nm");
      std::string line;
      while (std::getline(nmFile, line))
        if (line.find(" T main") != std::string::npos ||
            line.find(" T _main") != std::string::npos)
        {
          config->targetType = TargetType::Executable;
        }
    }
  }

  std::string objList;
  for (auto &resolver: resolverList)
    objList += resolver->fileName + " ";
  try
  {
    std::ostringstream strm;
    if (config->targetType == TargetType::Executable || config->targetType == TargetType::SharedLib)
    {
      strm << "g++";
      if (config->targetType == TargetType::SharedLib)
        strm << " -shared";
      strm << " " << objList << "-o " << fileName;
      for (const auto &flag: config->ldflags)
        strm << " " << flag;
      if (config->multithread && std::find(std::begin(config->ldflags), std::end(config->ldflags), "-pthread") == std::end(config->ldflags))
        strm << " -pthread";
      for (const auto &lib: config->libs)
        strm << " -l" << lib;
      if (!config->pkgs.empty())
      {
        strm << " $(pkg-config --libs";
        for (const auto &pkg: config->pkgs)
          strm << " " << pkg;
        strm << ")";
      }
    }
    else if (config->targetType == TargetType::StaticLib)
    {
      strm <<
        "ar rv " << fileName << " " << objList << std::endl <<
        "mkdir -p ~/.coddle/lib/\n"
        "ln -sf $(pwd)/" << fileName << " ~/.coddle/lib/";
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
