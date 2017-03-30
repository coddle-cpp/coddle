#include "vs_binary.hpp"
#include "config.hpp"
#include "error.hpp"
#include "osal.hpp"
#include "vs_object.hpp"
#include <iostream>
#include <sstream>

namespace Vs
{
void Binary::resolve()
{
  if (resolverList.empty())
  {
    std::cerr << "Nothing to build\n";
    return;
  }

  std::string objList;
  bool hasMain = false;
  for (auto &resolver: resolverList)
  {
    if (auto object = dynamic_cast<Object *>(resolver.get()))
      if (object->hasMain())
        hasMain = true;
    objList += resolver->fileName + " ";
  }
  try
  {
    std::ostringstream strm;
    if (hasMain)
    {
      strm << "cl";
      auto ldflags = Config::merge(config->common.ldflags, project->ldflags);
      for (const auto &flag: ldflags)
        strm << " " << flag;
      auto libs = Config::merge(config->common.libs, project->libs);
      for (const auto &lib: libs)
        strm << " -l" << lib;
      strm << " " << objList << "/link /out:" << fileName;
    }
    else
    {
      auto libName = "lib" + fileName + ".a";
      strm <<
        "ar rv " << libName << " " << objList;
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
