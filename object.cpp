#include "object.hpp"
#include "config.hpp"
#include "error.hpp"
#include "osal.hpp"
#include "exec_pool.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

#ifndef _WIN32
Object::Object(const std::string &source, Config *config):
  Dependency(".coddle" + getDirSeparator() + source + ".o", config),
  source(source)
{
}

void Object::job()
{
  try
  {
    {
      std::ostringstream strm;
      strm << "g++";
      for (const auto &flag: config->cflags)
        strm << " " << flag;
      if (!config->pkgs.empty())
      {
        strm << " $(pkg-config --cflags";
        for (const auto &pkg: config->pkgs)
          strm << " " << pkg;
        strm << ")";
      }
      strm << " -c " << source <<
        " -o " << fileName;
      std::cout << strm.str() << std::endl;
      strm << " -MT " << fileName <<
        " -MMD -MF " << fileName << ".mk";
      exec(strm.str());
    }
    {
      std::ostringstream strm;
      strm << "nm " << fileName << " > " << fileName << ".nm";
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
  std::ifstream nmFile(fileName + ".nm");
  std::string line;
  while (std::getline(nmFile, line))
    if (line.find(" T main") != std::string::npos ||
        line.find(" T _main") != std::string::npos)
      return true;
  return false;
}

#else
Object::Object(const std::string &source, Config *config):
  Dependency(".coddle" + getDirSeparator() + source + ".obj", config),
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
#endif

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
