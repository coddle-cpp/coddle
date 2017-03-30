#include "gcc_object.hpp"
#include "config.hpp"
#include "error.hpp"
#include "exec_pool.hpp"
#include "make_path.hpp"
#include "osal.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Gcc
{
Object::Object(const std::string &source, Config *config, ProjectConfig *project):
  Resolver(makePath(".coddle", source + ".o"), config, project),
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
      if (project->targetType == TargetType::SharedLib)
        strm << " -fPIC";
      auto cflags = Config::merge(config->common.cflags, project->cflags);
      for (const auto &flag: cflags)
        strm << " " << flag;
      if (config->multithread && std::find(std::begin(cflags), std::end(cflags), "-pthread") == std::end(cflags))
        strm << " -pthread";
      auto pkgs = Config::merge(config->common.pkgs, project->pkgs);
      if (!pkgs.empty())
      {
        strm << " $(pkg-config --cflags";
        for (const auto &pkg: pkgs)
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
      std::string str = [](const std::string &fileName)
        {
          std::ifstream f(fileName + ".mk");
          std::ostringstream strm;
          f >> strm.rdbuf();
          return strm.str();
        }(fileName);
      remove((fileName + ".mk").c_str());
      for (;;)
      {
        auto p = str.find("\\\n");
        if (p == std::string::npos)
          break;
        str.replace(p, 2, "");
      }
      for (;;)
      {
        auto p = str.find("\n");
        if (p == std::string::npos)
          break;
        str.replace(p, 1, "");
      }
      auto p = str.find(": ");
      str.replace(0, p + 2, "");
      std::istringstream strm(str);
      std::string header;
      std::ofstream hs(fileName.substr(0, fileName.size() - 2) + ".hs");
      while (std::getline(strm, header, ' '))
        if (!header.empty())
          hs << header << std::endl;
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
