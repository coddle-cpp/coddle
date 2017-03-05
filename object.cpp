#include "object.hpp"
#include "config.hpp"
#include "error.hpp"
#include "exec.hpp"
#include "exec_pool.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Object::Object(const std::string &source, Config *config):
  Dependency(".coddle/" + source + ".o", config),
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

void Object::resolve()
{
  resolved = false;
  ExecPool::instance().submit(std::bind(&Object::job, this));
}

bool Object::hasMain() const
{
  std::ifstream nmFile(fileName + ".nm");
  std::string line;
  while (std::getline(nmFile, line))
    if (line.find(" T main") != std::string::npos)
      return true;
  return false;
}

void Object::wait()
{
  std::unique_lock<std::mutex> l(mutex);
  while (!resolved)
    cond.wait(l);
  if (!errorString.empty())
    ERROR(errorString << std::endl);
}
