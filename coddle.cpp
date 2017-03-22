#include "coddle.hpp"
#include "config.hpp"
#include "exec_pool.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "file_name.hpp"
#include "osal.hpp"
#include "source.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef _WIN32

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
      config->configureForConfig();
    }
    makeDir(".coddle");
    auto root = config->binaryFactory(config);
    auto filesList = getFilesList(".");
    for (const auto &d: filesList)
    {
      auto ext = getFileExtention(d);
      if (ext != "c" &&
          ext != "cpp" &&
          ext != "c++" &&
          ext != "C" &&
          ext != "h" &&
          ext != "hpp" &&
          ext != "h++" &&
          ext != "H")
      {
        continue;
      }
      std::ifstream srcFile(d);
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
              if (ch == '\\')
                ch = '/';
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
      auto obj = root->add(config->objectFactory(d, config));
      obj->add(std::make_unique<Source>(config->execPath(), config));
      config->addDependency(obj, d);
    }
    try
    {
      root->resolveTree();
    }
    catch (...)
    {
      ExecPool::instance(config).finalize();
      throw;
    }
    if (!root->isRunResolve())
      std::cout << "coddle: '" << root->fileName << "' is up to date.\n";
    if (configDir)
    {
      std::cout << "coddle: Leaving directory `coddle.cfg'" << std::endl;
      changeDir("..");
      exec(std::string("coddle.cfg") + getDirSeparator() + "coddle");
    }
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}

#else
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
    auto target = config->target.empty() ? fileName(getCurrentWorkingDir()) + ".exe" : config->target;
    Binary root(target, config);
    auto filesList = getFilesList(".");
    for (const auto &d: filesList)
    {
      auto ext = getFileExtention(d);
      if (ext != "c" &&
          ext != "cpp" &&
          ext != "c++" &&
          ext != "C" &&
          ext != "h" &&
          ext != "hpp" &&
          ext != "h++" &&
          ext != "H")
      {
        continue;
      }
      std::ifstream srcFile(d);
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
              if (ch == '\\')
                ch = '/';
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
      auto obj = root.add(std::make_unique<Object>(d, config));
      obj->add(std::make_unique<Source>(config->execPath(), config));
      obj->add(std::make_unique<Source>(d, config));
      if (isFileExist(".coddle" + getDirSeparator() + d + ".obj.inc"))
      {
        std::ifstream f(".coddle" + getDirSeparator() + d + ".obj.inc");
        std::string header;
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
            obj->add(std::make_unique<Source>(header, config));
        }
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
      exec(std::string("coddle.cfg") + getDirSeparator() + "coddle");
    }
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}

#endif
