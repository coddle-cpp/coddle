#include "coddle.hpp"
#include "binary.hpp"
#include "current_path.hpp"
#include "dir.hpp"
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

void coddle()
{
  makeDir(".coddle");
  {
    Binary root(fileName(currentPath()));
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
      auto obj = root.add(std::make_unique<Object>(d.name()));
      if (!isFileExist(".coddle/" + d.name() + ".o.mk"))
        obj->add(std::make_unique<Source>(d.name()));
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
            obj->add(std::make_unique<Source>(srcFile));
      }
    }
    root.resolveTree();
  }
}
