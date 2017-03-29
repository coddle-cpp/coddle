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
    auto root = config->driver->makeBinaryResolver(config);
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
      auto obj = root->add(config->driver->makeObjectResolver(d, config));
      obj->add(std::make_unique<Source>(config->execPath(), config));
      obj->add(std::make_unique<Source>(d, config));
      std::ifstream hs(".coddle" + getDirSeparator() + d + ".hs");
      std::string header;
      while (std::getline(hs, header))
        if (!header.empty())
          obj->add(std::make_unique<Source>(header, config));
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
