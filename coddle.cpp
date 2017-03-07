#include "coddle.hpp"
#include "binary.hpp"
#include "config.hpp"
#include "osal.hpp"
#include "dir.hpp"
#include "osal.hpp"
#include "exec_pool.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "file_name.hpp"
#include "osal.hpp"
#include "object.hpp"
#include "source.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

int coddle(Config *config)
{
  try
  {
    bool configDir = false;
    if (!config->configured() && isFileExist("coddle.cfg"))
    {
      configDir = true;
      std::cout << "coddle: Entering directory `coddle.cfg'" << std::endl;
      changeDir("coddle.cfg");
      config->target = "coddle";
      config->cflags.push_back("-I ~/.coddle/include");
      config->ldflags.push_back("-L ~/.coddle/lib");
      config->ldflags.push_back("-lcoddle");
      if (std::find(std::begin(config->cflags), std::end(config->cflags), "-pthread") == std::end(config->cflags))
        config->cflags.push_back("-pthread");
      if (std::find(std::begin(config->ldflags), std::end(config->ldflags), "-pthread") == std::end(config->ldflags))
        config->ldflags.push_back("-pthread");
    }
    makeDir(".coddle");
    auto target = config->target.empty() ? fileName(currentPath()) : config->target;
    Binary root(target, config);
    for (const auto &d: Dir("."))
    {
      if (d.type() != Dir::Entry::Regular &&
          d.type() != Dir::Entry::Link)
        continue;
      auto ext = getFileExtention(d.name());
      if (ext != "c" &&
          ext != "cpp" &&
          ext != "c++" &&
          ext != "C" &&
          ext != "h" &&
          ext != "hpp" &&
          ext != "h++" &&
          ext != "H"
          )
      {
        continue;
      }
      std::ifstream srcFile(d.name());
      std::string line;
      while (std::getline(srcFile, line))
      {
        line = [](const std::string &x)
          {
            std::string res;
            for (auto ch: x)
            {
              if (ch <= ' ' && ch >= 0)
                continue;
              res += ch;
            }
            return res;
          }(line);
        if (line.find("#include<") != 0)
          continue;
        auto p = line.find(">");
        if (p != line.size() - 1)
          continue;
        auto header = line.substr(9);
        header.resize(header.size() - 1);
        {
          auto iter = config->incToPkg.find(header);
          if (iter != std::end(config->incToPkg))
            for (const auto &lib: iter->second)
              if (std::find(std::begin(config->pkgs), std::end(config->pkgs), lib) == std::end(config->pkgs))
                config->pkgs.push_back(lib);
        }
        {
          auto iter = config->incToLib.find(header);
          if (iter != std::end(config->incToLib))
            for (const auto &lib: iter->second)
              if (std::find(std::begin(config->libs), std::end(config->libs), lib) == std::end(config->libs))
                config->libs.push_back(lib);
        }
      }
      if (ext != "c" &&
          ext != "cpp" &&
          ext != "c++" &&
          ext != "C")
      {
        continue;
      }
      auto obj = root.add(std::make_unique<Object>(d.name(), config));
      obj->add(std::make_unique<Source>(config->execPath(), config));
      if (!isFileExist(".coddle/" + d.name() + ".o.mk"))
        obj->add(std::make_unique<Source>(d.name(), config));
      else
      {
        std::string str = [](const std::string &file)
          {
            std::ifstream f(".coddle/" + file + ".o.mk");
            std::ostringstream strm;
            f >> strm.rdbuf();
            return strm.str();
          }(d.name());
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
        std::string srcFile;
        while (std::getline(strm, srcFile, ' '))
          if (!srcFile.empty())
            obj->add(std::make_unique<Source>(srcFile, config));
      }
    }
    try
    {
      root.resolveTree();
    }
    catch (...)
    {
      ExecPool::instance(config).finalize();
      throw;
    }
    if (!root.isRunResolve())
      std::cout << "coddle: '" << root.fileName << "' is up to date.\n";
    if (configDir)
    {
      std::cout << "coddle: Leaving directory `coddle.cfg'" << std::endl;
      changeDir("..");
      exec("coddle.cfg/coddle");
    }
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
