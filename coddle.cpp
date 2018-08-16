#include "coddle.hpp"
#include "config.hpp"
#include "dependency_tree.hpp"
#include "file_exist.hpp"
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

static void srcToLibs(const std::string &fileName,
                      const std::unordered_map<std::string, const Library *> &incToLib,
                      const Config &config)
{
  std::ifstream srcFile(config.srcDir + "/" + fileName);
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
  std::unordered_set<std::string> localLibs;
  for (const auto &header : headerList)
  {
    auto it = incToLib.find(header);
    if (it == std::end(incToLib))
      continue;
    localLibs.insert(it->second->name);
  }
  std::ofstream libsFile(config.artifactsDir + "/" + fileName + ".libs");
  for (const auto &lib : localLibs)
  {
    if (lib.empty())
      continue;
    libsFile << lib << std::endl;
  }
}

int Coddle::exec(const Config &config)
{
  std::unordered_set<std::string> localLibs;
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
      for (auto &&fileName : getFilesList(config.srcDir))
      {
        static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
        static std::unordered_set<std::string> headerExtentions = {"h", "hpp", "h++", "H"};
        auto &&extention = getFileExtention(fileName);
        if (srcExtentions.find(extention) != std::end(srcExtentions) ||
            headerExtentions.find(extention) != std::end(headerExtentions))
        {
          libsFiles.push_back(config.artifactsDir + "/" + fileName + ".libs");
          auto &&dependency =
            dependencyTree.addTarget(config.artifactsDir + "/" + fileName + ".libs");
          dependency->dependsOf(config.srcDir + "/" + fileName);
          dependency->dependsOf(".coddle/remote/libraries.toml");
          dependency->dependsOf(config.localRepository + "/libraries.toml");

          dependency->exec = [fileName, &config, &incToLib]() {
            srcToLibs(fileName, incToLib, config);
          };
        };
      }
      dependencyTree.resolve();
    }

    // generate libs file for whole project
    std::string libsStr = [&]() {
      for (auto &&libsFile : libsFiles)
      {
        std::ifstream strm(libsFile);
        std::string lib;
        while (std::getline(strm, lib))
          localLibs.insert(lib);
      }

      std::ostringstream libsStrm;
      for (auto &&lib : localLibs)
        libsStrm << lib << std::endl;
      return libsStrm.str();
    }();

    std::string oldLibs = [&]() {
      std::ifstream libsFile(config.artifactsDir + "/libs");
      std::ostringstream buffer;
      buffer << libsFile.rdbuf();
      return buffer.str();
    }();

    if (oldLibs != libsStr)
    {
      std::ofstream libsFile(config.artifactsDir + "/libs");
      libsFile << libsStr;
    }
  }

  { // download and build libraries
    makeDir(".coddle/libs_src");
    for (auto &&libName : localLibs)
    {
      auto it = repository.libraries.find(libName);
      if (it == std::end(repository.libraries))
        throw std::runtime_error("Library is not found: " + libName);
      auto &&lib = it->second;
      auto repoDir = ".coddle/libs_src/" + lib.name;
      switch (lib.type)
      {
      case Library::Type::File:
        if (!isDirExist(repoDir))
          execShowCmd("ln -s", lib.path, repoDir);
        break;
      case Library::Type::Git:
        if (!isDirExist(repoDir))
        {
          execShowCmd("git clone --depth 1", lib.path, "-b", lib.version, repoDir);
         if (!lib.postClone.empty())
           execShowCmd("cd", repoDir, "&&", lib.postClone);
        }

        break;
      case Library::Type::PkgConfig:
        pkgs.insert(lib.name);
        break;
      case Library::Type::Lib:
        libs.insert(lib.name);
        break;
      }

      if (libs.find(lib.name) == std::end(libs) && lib.name != config.target &&
          (lib.type == Library::Type::File || lib.type == Library::Type::Git))
      {
        auto libConfig = config;
        libConfig.srcDir = repoDir;
        makeDir(".coddle/a");
        libConfig.targetDir = ".coddle/a";
        makeDir(".coddle/libs_artifacts/" + lib.name);
        libConfig.artifactsDir = ".coddle/libs_artifacts/" + lib.name;
        libConfig.target = lib.name;
        this->exec(libConfig);
        libs.insert(lib.name);
      }
    }
  }

  { // generate cflags file
    auto cflagsStr = [&]() {
      std::ostringstream cflags;
      // packages
      if (!pkgs.empty())
      {
        cflags << " $(pkg-config --cflags";
        for (const auto &pkg : pkgs)
          cflags << " " << pkg;
        cflags << ")";
      }
      cflags << " " << config.cflags;
      if (config.debug)
        cflags << " -g -O0";
      else
        cflags << " -O3";
      return cflags.str();
    }();
    std::string oldCflags = [&]() {
      std::ifstream cflagsFile(config.artifactsDir + "/cflags");
      std::ostringstream buffer;
      buffer << cflagsFile.rdbuf();
      return buffer.str();
    }();
    if (oldCflags != cflagsStr)
    {
      std::ofstream cflagsFile(config.artifactsDir + "/cflags");
      cflagsFile << cflagsStr;
    }
  }

  // get list of source files
  std::vector<std::string> srcFiles;
  {
    for (auto &&fileName : getFilesList(config.srcDir))
    {
      auto ext = getFileExtention(fileName);
      static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
      auto &&extention = getFileExtention(fileName);
      if (srcExtentions.find(extention) != std::end(srcExtentions))
        srcFiles.push_back(fileName);
    }
  }

  {
    DependencyTree dependencyTree;
    for (auto &&fileName : srcFiles)
    {
      { // compile source file
        auto &&dependency = dependencyTree.addTarget(config.artifactsDir + "/" + fileName + ".o");
        std::ifstream headers(config.artifactsDir + "/" + fileName + ".headers");
        if (!headers)
          dependency->dependsOf(config.srcDir + "/" + fileName);
        else
        {
          std::string header;
          while (std::getline(headers, header))
            dependency->dependsOf(header);
        }
        dependency->dependsOf(config.artifactsDir + "/libs");
        dependency->dependsOf(config.artifactsDir + "/cflags");
        dependency->exec = [fileName, &config]() {
          std::ostringstream cmd;

          cmd << "clang++";
          { // load cflags
            std::ifstream cflags(config.artifactsDir + "/cflags");
            if (cflags)
              cmd << cflags.rdbuf();
          }
          // include dirs
          cmd << " -I.coddle/libs_src";

          cmd << " -c " << config.srcDir << "/" << fileName << " -o " << config.artifactsDir << "/"
              << fileName + ".o";
          std::cout << cmd.str() << std::endl;
          cmd << " -MT " << config.srcDir << "/" << fileName << " -MMD -MF " << config.artifactsDir
              << "/" << fileName << ".mk";
          try
          {
            ::exec(cmd.str());
            { // generate .headers
              std::string mk = [&fileName, &config]() {
                std::ifstream f(config.artifactsDir + "/" + fileName + ".mk");
                std::ostringstream strm;
                f >> strm.rdbuf();
                return strm.str();
              }();
              remove((config.artifactsDir + "/" + fileName + ".mk").c_str());
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
              std::ofstream headers(config.artifactsDir + "/" + fileName + ".headers");
              while (std::getline(strm, header, ' '))
                if (!header.empty())
                  headers << header << std::endl;
            }
          }
          catch (std::exception &e)
          {
            std::cout << "coddle: *** [" + config.srcDir + "/" + fileName + "] " + e.what()
                      << std::endl;
          }
        };
      }

      { // determine is the project executable or library
        auto &&dependency =
          dependencyTree.addTarget(config.artifactsDir + "/" + fileName + ".hasmain");
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
        dependency->exec = [fileName, &config]() {
          std::ostringstream cmd;
          cmd << "nm " << config.artifactsDir << "/" << fileName << ".o"
              << " > " << config.artifactsDir << "/" << fileName << ".nm";
          ::exec(cmd.str());
          auto hasMain = false;
          { // parse .nm file
            std::ifstream nmFile(config.artifactsDir + "/" + fileName + ".nm");
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
            remove((config.artifactsDir + "/" + fileName + ".nm").c_str());
          }
          std::ofstream hasMainFile(config.artifactsDir + "/" + fileName + ".hasmain");
          hasMainFile << hasMain << std::endl;
        };
      }
    }

    dependencyTree.resolve();
  }

  { // linking
    auto isExec = false;
    for (auto &&fileName : srcFiles)
    {
      std::ifstream f(config.artifactsDir + "/" + fileName + ".hasmain");
      bool hasMain;
      f >> hasMain;
      if (hasMain)
      {
        isExec = true;
        break;
      }
    }
    DependencyTree dependencyTree;
    if (isExec)
    {
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/" + config.target);
      std::ostringstream strm;
      strm << "clang++";
      for (auto &&fileName : srcFiles)
      {
        strm << " " << config.artifactsDir << "/" << fileName << ".o";
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      }
      strm << " -o " << config.targetDir << "/" << config.target;

      strm << " -L.coddle/a";

      for (auto &&libName : libs)
      {
        auto it = repository.libraries.find(libName);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName);
        auto &&lib = it->second;
        strm << " -l" << lib.name;
        if (lib.type == Library::Type::File || lib.type == Library::Type::Git)
          dependency->dependsOf(".coddle/a/lib" + lib.name + ".a");
      }

      if (!pkgs.empty())
      {
        strm << " $(pkg-config --libs";
        for (const auto &pkg : pkgs)
          strm << " " << pkg;
        strm << ")";
      }

      // TODO ldflags
      dependency->dependsOf(config.artifactsDir + "/ldflags");
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    else
    {
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/lib" + config.target + ".a");
      std::ostringstream strm;
      strm << "ar r " << config.targetDir + "/lib" + config.target + ".a";
      for (auto &&fileName : srcFiles)
      {
        strm << " " << config.artifactsDir << "/" << fileName << ".o";
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      }
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    dependencyTree.resolve();
  }

  return 0;
}
