#include "coddle.hpp"
#include "binary.hpp"
#include "config.hpp"
#include "current_path.hpp"
#include "dir.hpp"
#include "exec.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "file_name.hpp"
#include "make_dir.hpp"
#include "object.hpp"
#include "source.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

int coddle(Config *config)
{
  try
  {
    bool configDir = false;
    if (!config->configured() && isFileExist("coddle.cfg"))
    {
      configDir = true;
      std::cout << "coddle: Entering directory `coddle.cfg'" << std::endl;
      chdir("coddle.cfg");
      config->target = "coddle";
      config->cflags.push_back("-I ~/.coddle/include");
      config->ldflags.push_back("-L ~/.coddle/lib");
      config->ldflags.push_back("-lcoddle");
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
          ext != "C")
      {
        continue;
      }
      auto obj = root.add(std::make_unique<Object>(d.name(), config));
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
    root.resolveTree();
    if (configDir)
    {
      std::cout << "coddle: Leving directory `coddle.cfg'" << std::endl;
      chdir("..");
      if (root.isRunResolve())
        exec("rm -rf .coddle");
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
