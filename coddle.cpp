#include "coddle.hpp"
#include "config.hpp"
#include "dependency_tree.hpp"
#include "file_extention.hpp"
#include "library.hpp"
#include "osal.hpp"
#include "repository.hpp"
#include "resolver.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

static std::string toLibsFile(const std::string &fileName) // TOOD remove
{
  return ".coddle/" + fileName + ".libs";
}

static void srcToLibs(const std::string &fileName,
                      const std::unordered_map<std::string, const Library *> incToLib)
{
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
  {
    auto it = incToLib.find(header);
    if (it == std::end(incToLib))
      continue;
    libs.insert(it->second->name);
  }
  std::ofstream libsFile(toLibsFile(fileName));
  for (const auto &lib : libs)
  {
    if (lib.empty())
      continue;
    libsFile << lib << std::endl;
  }
}

int coddle(const Config &config, const Repository &repository)
{
  { // generate .libs file
    std::unordered_map<std::string, const Library *> incToLib;
    for (auto &&lib : repository.libraries)
      for (auto &&inc : lib.second.includes)
        incToLib[inc] = &lib.second;
    // generate libs files for each source files
    std::vector<std::string> libsFiles;
    {
      DependencyTree dependencyTree;
      // get the list of .cpp files
      for (auto &&fileName : getFilesList("."))
      {
        static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
        static std::unordered_set<std::string> headerExtentions = {"h", "hpp", "h++", "H"};
        auto &&extention = getFileExtention(fileName);
        if (srcExtentions.find(extention) != std::end(srcExtentions) ||
            headerExtentions.find(extention) != std::end(headerExtentions))
        {
          libsFiles.push_back(toLibsFile(fileName));
          auto &&dependency = dependencyTree.addTarget(toLibsFile(fileName));
          dependency->dependsOf(fileName);
          dependency->dependsOf(".coddle/remote/libraries.toml");
          dependency->dependsOf(config.localRepository + "/libraries.toml");

          dependency->exec = [fileName, &incToLib]() { srcToLibs(fileName, incToLib); };
        };
      }
      dependencyTree.resolve();
    }

    // generate libs file for whole project
    std::string libsStr = [&]() {
      std::unordered_set<std::string> libs;
      for (auto &&libsFile : libsFiles)
      {
        std::ifstream strm(libsFile);
        std::string lib;
        while (std::getline(strm, lib))
          libs.insert(lib);
      }

      std::ostringstream libsStrm;
      for (auto &&lib : libs)
        libsStrm << lib << std::endl;
      return libsStrm.str();
    }();

    std::string oldLibs = [&]() {
      std::ifstream libsFile(".coddle/libs");
      std::ostringstream buffer;
      buffer << libsFile.rdbuf();
      return buffer.str();
    }();

    if (oldLibs != libsStr)
    {
      std::ofstream libsFile(".coddle/libs");
      libsFile << libsStr;
    }
  }

  { // TODO download libraries

  }

  { // TODO generate .cflags file
    
  }

  // get list of source files
  std::vector<std::string> srcFiles;
  {
    for (auto &&fileName : getFilesList("."))
    {
      auto ext = getFileExtention(fileName);
      static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
      auto &&extention = getFileExtention(fileName);
      if (srcExtentions.find(extention) != std::end(srcExtentions))
      {
        srcFiles.push_back(fileName);
      }
    }
  }
  
  {
    DependencyTree dependencyTree;
    for (auto &&fileName : srcFiles)
    {
      { // compile source file
        auto &&dependency = dependencyTree.addTarget(".coddle/" + fileName + ".o");
        std::ifstream headers(".coddle/" + fileName + ".headers");
        if (!headers)
          dependency->dependsOf(fileName);
        else
        {
          std::string header;
          while (std::getline(headers, header))
            dependency->dependsOf(header);
        }
        dependency->dependsOf(".coddle/libs");
        dependency->dependsOf(".coddle/cflags");
        dependency->exec = [fileName]() {
          std::ostringstream cmd;

          cmd << "clang++";
          std::ifstream cflags(".coddle/cflags");
          if (cflags)
            cmd << " " << cflags.rdbuf();
          // TODO include dirs
          // TODO packages
          cmd << " -c " << fileName << " -o "
              << ".coddle/" << fileName + ".o";
          std::cout << cmd.str() << std::endl;
          cmd << " -MT " << fileName << " -MMD -MF "
              << ".coddle/" << fileName << ".mk";
          try
          {
            exec(cmd.str());
            { // generate .headers
              std::string mk = [&fileName]() {
                std::ifstream f(".coddle/" + fileName + ".mk");
                std::ostringstream strm;
                f >> strm.rdbuf();
                return strm.str();
              }();
              remove((".coddle/" + fileName + ".mk").c_str());
              for (;;)
              {
                auto p = mk.find("\\\n");
                if (p == std::string::npos)
                  break;
                mk.replace(p, 2, "");
              }
              for (;;)
              {
                auto p = mk.find("\n");
                if (p == std::string::npos)
                  break;
                mk.replace(p, 1, "");
              }
              auto p = mk.find(": ");
              mk.replace(0, p + 2, "");
              std::istringstream strm(mk);
              std::string header;
              std::ofstream headers(".coddle/" + fileName + ".headers");
              while (std::getline(strm, header, ' '))
                if (!header.empty())
                  headers << header << std::endl;
            }
          }
          catch (std::exception &e)
          {
            std::cout << "coddle: *** [" + fileName + "] " + e.what() << std::endl;
          }
        };
      }

      { // determine is the project executable or library
        auto &&dependency = dependencyTree.addTarget(".coddle/" + fileName + ".hasmain");
        dependency->dependsOf(".coddle/" + fileName + ".o");
        dependency->exec = [fileName]() {
          std::ostringstream cmd;
          cmd << "nm "
              << ".coddle/" << fileName << ".o"
              << " > "
              << ".coddle/" << fileName << ".nm";
          exec(cmd.str());
          auto hasMain = false;
          { // parse .nm file
            std::ifstream nmFile(".coddle/" + fileName + ".nm");
            std::string line;
            while (std::getline(nmFile, line))
            {
              if (line.find(" T main") != std::string::npos ||
                  line.find(" T _main") != std::string::npos)
              {
                hasMain = true;
                break;
              }
            }
            remove((".coddle/" + fileName + ".nm").c_str());
          }
          std::ofstream hasMainFile(".coddle/" + fileName + ".hasmain");
          hasMainFile << hasMain << std::endl;
        };
      }
    }

    dependencyTree.resolve();
  }

  { // linking
    auto isExec = false;
    for (auto &&fileName : getFilesList(".coddle"))
    {
      auto &&extention = getFileExtention(fileName);
      if (extention == "hasmain")
      {
        std::ifstream f(".coddle/" + fileName);
        bool hasMain;
        f >> hasMain;
        if (hasMain)
        {
          isExec = true;
          break;
        }
      }
    }
    DependencyTree dependencyTree;
    if (isExec)
    {
      auto &&dependency = dependencyTree.addTarget(config.target);
      std::ostringstream strm;
      strm << "clang++";
      for (auto &&fileName : srcFiles)
      {
        strm << " "
            << ".coddle/" << fileName << ".o";
        dependency->dependsOf(".coddle/" + fileName + ".o");
      }
      strm << " -o " << config.target;
      // TODO ldflags
      dependency->dependsOf(".coddle/ldflags");
      auto cmd = strm.str();
      dependency->exec = [cmd]()
        {
          std::cout << cmd << std::endl;
          exec(cmd);
        };
    }
    dependencyTree.resolve();
  }

  return 0;
}
