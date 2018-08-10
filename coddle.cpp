#include "coddle.hpp"
#include "resolver.hpp"
#include "dependency_tree.hpp"
#include "file_extention.hpp"
#include "library.hpp"
#include "osal.hpp"
#include "repository.hpp"
#include <fstream>
#include <unordered_map>
#include <unordered_set>

int coddle(const Config &config, const Repository &repository)
{
  std::unordered_map<std::string, const Library *> incToLib;
  for (auto &&lib : repository.libraries)
    for (auto &&inc : lib.second.includes)
      incToLib[inc] = &lib.second;

  DependencyTree dependencyTree;

  // get the list of .cpp files
  for (auto &&fileName : getFilesList("."))
  {
    auto ext = getFileExtention(fileName);
    static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
    static std::unordered_set<std::string> headerExtentions = {"h", "hpp", "h++", "H"};
    auto &&extention = getFileExtention(fileName);
    if (srcExtentions.find(extention) != std::end(srcExtentions) ||
        headerExtentions.find(extention) != std::end(headerExtentions))
    {
      auto &&dependency = dependencyTree.addTarget(".coddle/" + fileName + ".libs");
      dependency->dependsOf(fileName);
      dependency->exec = [fileName, &incToLib]() {
        std::ifstream srcFile(fileName);
        std::string line;
        std::unordered_set<std::string> headerList;
        while (std::getline(srcFile, line))
        {
          line = [](const std::string &x) {
            std::string res;
            for (auto ch : x)
            {
              if (ch <= ' ' && ch >= 0)
                continue;
              if (ch == '\\')
                ch = '/';
              res += ch;
            }
            return res;
          }(line);
          if (line.find("#include") == 0)
          {
            auto p = line.find_last_of("\">");
            if (p == line.size() - 1)
            {
              auto header = line.substr(9);
              header.resize(header.size() - 1);
              headerList.insert(header);
            }
          }

          if (line.find("#include<") != 0)
            continue;
          auto p = line.find(">");
          if (p != line.size() - 1)
            continue;
          auto header = line.substr(9);
          header.resize(header.size() - 1);
          headerList.insert(header);
        }
        std::unordered_set<std::string> libs;
        for (const auto &header : headerList)
          libs.insert(incToLib[header]->name);
        std::ofstream libsFile(".coddle/" + fileName + ".libs");
        for (const auto &lib : libs)
        {
          if (lib.empty())
            continue;
          libsFile << lib << std::endl;
        }
      };
    };
  }
  dependencyTree.resolve();
  return 0;
}
