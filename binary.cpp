#include "binary.hpp"
#include "config.hpp"
#include "error.hpp"
#include "exec.hpp"
#include "object.hpp"
#include <iostream>
#include <sstream>

void Binary::resolve()
{
  if (dependencyList.empty())
  {
    std::cerr << "Nothing to build\n";
    return;
  }

  std::string objList;
  bool hasMain = false;
  for (auto &dependency: dependencyList)
  {
    if (auto object = dynamic_cast<Object *>(dependency.get()))
      if (object->hasMain())
        hasMain = true;
    objList += dependency->fileName + " ";
  }
  try
  {
    std::ostringstream strm;
    if (hasMain)
    {
      strm << "g++";
      strm << " " << objList << "-o " << fileName;
      for (const auto &flag: config->ldflags)
        strm << " " << flag;
      if (!config->pkgs.empty())
      {
        strm << " $(pkg-config --libs";
        for (const auto &pkg: config->pkgs)
          strm << " " << pkg;
        strm << ")";
      }
    }
    else
    {
      auto libName = "lib" + fileName + ".a";
      strm <<
        "ar rv " << libName << " " << objList << std::endl <<
        "mkdir -p ~/.coddle/lib/\n"
        "ln -sf `realpath .`/" << libName << " ~/.coddle/lib/";
    }
    std::cout << strm.str() << std::endl;
    exec(strm.str());
  }
  catch (std::exception &e)
  {
    ERROR("coddle: *** [" << fileName << "] " << e.what())
  }
}
