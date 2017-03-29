#include "vs_object.hpp"
#include "config.hpp"
#include "error.hpp"
#include "osal.hpp"
#include "exec_pool.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace Vs
{
Object::Object(const std::string &source, Config *config):
  Resolver(".coddle" + getDirSeparator() + source + ".obj", config),
  source(source)
{
}

void Object::job()
{
  try
  {
    {
      std::ostringstream strm;
      strm << "cl";
      for (const auto &flag: config->cflags)
        strm << " " << flag;
      strm << " /showIncludes /Zs " << source <<
        " > " << fileName << ".inc";
      exec(strm.str());
      std::ifstream f(fileName + ".inc");
      std::string header;
      std::ofstream hs(fileName.substr(0, fileName.size() - 4) + ".hs");
      while (std::getline(f, header))
      {
        auto p = header.find(":");
        if (p == std::string::npos)
          continue;
        p = header.find(":", p + 1);
        if (p == std::string::npos)
          continue;
        ++p;
        while (p < header.size() && header[p] == ' ')
          ++p;
        header = header.substr(p);
        if (!header.empty())
          hs << header << std::endl;
      }
    }
    {
      std::ostringstream strm;
      strm << "cl";
      for (const auto &flag: config->cflags)
        strm << " " << flag;
      strm << " /Fo" << fileName << " /c " << source;
      std::cout << strm.str() << std::endl;
      exec(strm.str());
    }
  }
  catch (std::exception &e)
  {
    errorString = "coddle: *** [" + fileName + "] " + e.what();
  }
  catch (...)
  {
    errorString = "coddle: *** [" + fileName + "] Unknown error";
  }
  std::lock_guard<std::mutex> l(mutex);
  resolved = true;
  cond.notify_all();
}

bool Object::hasMain() const
{
  return true;
}

void Object::resolve()
{
  resolved = false;
  ExecPool::instance(config).submit(std::bind(&Object::job, this));
}

void Object::wait()
{
  std::unique_lock<std::mutex> l(mutex);
  while (!resolved)
    cond.wait(l);
  if (!errorString.empty())
    THROW_ERROR(errorString << std::endl);
}
}
