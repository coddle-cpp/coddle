#include "coddle.hpp"
#include "config.hpp"
#include "dependency_tree.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "library.hpp"
#include "osal.hpp"
#include "path_relative.hpp"
#include "repository.hpp"
#include "resolver.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using IncToLib = std::unordered_map<std::string, std::vector<const Library *>>;

static std::string makeLibsFile(const std::string& fileName, const Config& config)
{
  return config.artifactsDir + "/" + fileName + ".libs";
}

// parses the file, derivies libraries from the file and saves result to
// the config.artifactsDir directory
static void srcToLibs(const std::string &fileName,
                      const IncToLib &incToLib,
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
    for (auto &&lib : it->second)
      localLibs.insert(lib->name);
  }
  std::ofstream libsFile(makeLibsFile(fileName, config));
  for (const auto &lib : localLibs)
  {
    if (lib.empty())
      continue;
    libsFile << lib << std::endl;
  }
}

// generates *.libs file for each source and header file and generates
// [target].libs file
std::unordered_set<std::string> Coddle::generateLibsFiles(const Config &config) const
{
  std::unordered_set<std::string> localLibs;
  const auto incToLib = [&]() {
    IncToLib ret;
    for (auto &&lib : repository.libraries)
      for (auto &&inc : lib.second.includes)
        ret[inc].push_back(&lib.second);
    return ret;
  }();
  // generate libs files for each source files
  std::vector<std::string> libsFiles;
  makeDir(config.artifactsDir);
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
      libsFiles.push_back(makeLibsFile(fileName, config));
      auto &&dependency = dependencyTree.addTarget(makeLibsFile(fileName, config));
      dependency->dependsOf(config.srcDir + "/" + fileName);
      dependency->dependsOf(".coddle/remote/libraries.toml");
      dependency->dependsOf(config.localRepository + "/libraries.toml");

      dependency->exec = [fileName, &config, &incToLib]() {
        srcToLibs(fileName, incToLib, config);
      };
    };
  }
  dependencyTree.resolve();

  for (auto &&libsFile : libsFiles)
  {
    std::ifstream strm(libsFile);
    std::string lib;
    while (std::getline(strm, lib))
      localLibs.insert(lib);
  }

  return localLibs;
}

// generate libs file for whole project
void Coddle::generateProjectLibsFile(const Config &config) const
{
  std::string libsStr = [&]() {
    std::ostringstream libsStrm;
    for (auto &&lib : libs)
      libsStrm << lib << std::endl;
    for (auto &&pkg : pkgs)
      libsStrm << pkg << std::endl;
    return libsStrm.str();
  }();

  // if libs file did not change we are not updating the file on the disk
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

bool Coddle::downloadAndBuildLibs(const Config &config,
                                  const std::unordered_set<std::string> &localLibs)
{
  auto hasNativeLibs{false};
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
      {
        makeDir(".coddle/libs_src");
        execShowCmd("ln -s",
                    (isPathRelative(lib.path) ? (getCurrentWorkingDir() + "/") : "") + lib.path,
                    repoDir);
      }
      break;
    case Library::Type::Git:
      if (!isDirExist(repoDir))
      {
        makeDir(".coddle/libs_src");
        execShowCmd("git clone --depth 1", lib.path, "-b", lib.version, repoDir);
        if (!lib.postClone.empty())
          execShowCmd("cd", repoDir, "&&", lib.postClone);
      }
      break;
    case Library::Type::PkgConfig: pkgs.insert(lib.name); break;
    case Library::Type::Lib: libs.insert(lib.name); break;
    }

    if (libs.find(lib.name) == std::end(libs) && lib.name != config.target &&
        (lib.type == Library::Type::File || lib.type == Library::Type::Git))
    {
      hasNativeLibs = true;
      auto libConfig = config;
      libConfig.srcDir = repoDir;
      makeDir(".coddle/a");
      libConfig.targetDir = ".coddle/a";
      makeDir(".coddle/libs_artifacts/" + lib.name);
      libConfig.artifactsDir = ".coddle/libs_artifacts/" + lib.name;
      libConfig.target = lib.name;
      if (build(libConfig) > 0)
        libs.insert(lib.name);
    }
  }
  return hasNativeLibs;
}

int Coddle::build(const Config &config)
{
  std::unordered_set<std::string> localLibs = generateLibsFiles(config);
  auto hasNativeLibs = downloadAndBuildLibs(config, localLibs);
  generateProjectLibsFile(config);

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

      for (auto &&libName : localLibs)
      {
        auto it = repository.libraries.find(libName);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName);
        auto &&lib = it->second;
        if (!lib.incdir.empty())
          cflags << " -I.coddle/libs_src/" << libName << "/" << lib.incdir;
      }

      cflags << " " << config.cflags;
      if (config.debug)
        cflags << " -g -O0";
      else
        cflags << " -O3";
      if (config.multithreaded)
        cflags << " -pthread";
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
        dependency->exec = [fileName, &config, hasNativeLibs]() {
          std::ostringstream cmd;

          cmd << "clang++";
          { // load cflags
            std::ifstream cflags(config.artifactsDir + "/cflags");
            if (cflags)
              cmd << cflags.rdbuf();
          }
          // include dirs
          if (hasNativeLibs)
            cmd << " -I.coddle/libs_src";

          cmd << " -c " << config.srcDir << "/" << fileName << " -o " << config.artifactsDir << "/"
              << fileName + ".o";
          std::cout << cmd.str() << std::endl;
          cmd << " -MT " << config.srcDir << "/" << fileName << " -MMD -MF " << config.artifactsDir
              << "/" << fileName << ".mk";
#ifdef _WIN32
          cmd << " -Xclang -flto-visibility-public-std";
#endif
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
        // TODO: use better heuristic
        auto &&dependency =
          dependencyTree.addTarget(config.artifactsDir + "/" + fileName + ".hasmain");
        dependency->dependsOf(fileName);
        dependency->exec = [fileName, &config]() {
          std::ostringstream cmd;
          auto hasMain = false;
          { // parse .nm file
            std::ifstream srcFile(fileName);
            std::string line;
            while (std::getline(srcFile, line))
            {
              if (line.find("int main(") == 0)
              {
                hasMain = true;
                break;
              }
            }
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
#ifndef _WIN32
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/" + config.target);
#else
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/" + config.target + ".exe");
#endif
      std::ostringstream strm;
      strm << "clang++";
      for (auto &&fileName : srcFiles)
      {
        strm << " " << config.artifactsDir << "/" << fileName << ".o";
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      }
#ifndef _WIN32
      strm << " -o " << config.targetDir << "/" << config.target;
#else
      strm << " -o " << config.targetDir << "/" << config.target << ".exe";
#endif

      if (hasNativeLibs)
        strm << " -L.coddle/a";

      for (auto &&libName : libs)
      {
        auto it = repository.libraries.find(libName);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName);
        auto &&lib = it->second;
        strm << " -l" << lib.name;
        if (lib.type == Library::Type::File || lib.type == Library::Type::Git)
#ifndef _WIN32
          dependency->dependsOf(".coddle/a/lib" + lib.name + ".a");
#else
          dependency->dependsOf(".coddle/a/" + lib.name + ".lib");
#endif
      }
      for (auto &&libName : localLibs)
      {
        auto it = repository.libraries.find(libName);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName);
        auto &&lib = it->second;
        if (!lib.libdir.empty())
          strm << " -L.coddle/libs_src/" << libName << "/" << lib.libdir;
      }

      if (!pkgs.empty())
      {
        strm << " $(pkg-config --libs";
        for (const auto &pkg : pkgs)
          strm << " " << pkg;
        strm << ")";
      }

#ifdef _WIN32
         strm << " -Xclang -flto-visibility-public-std";
#endif

      // TODO ldflags
      if (config.multithreaded)
        // FIXME: trigger reling if changed
        strm << " -pthread";
      dependency->dependsOf(config.artifactsDir + "/ldflags");
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    else if (!srcFiles.empty())
    {
#ifndef _WIN32
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/lib" + config.target + ".a");
      std::ostringstream strm;
      strm << "ar r " << config.targetDir + "/lib" + config.target + ".a";
      for (auto &&fileName : srcFiles)
      {
        strm << " " << config.artifactsDir << "/" << fileName << ".o";
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      }
#else
      auto &&dependency = dependencyTree.addTarget(config.targetDir + "/" + config.target + ".lib");
      std::ostringstream strm;
      strm << "lib.exe /OUT:" << config.targetDir + "/" + config.target + ".lib";
      for (auto &&fileName : srcFiles)
      {
        strm << " " << config.artifactsDir << "/" << fileName << ".o";
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      }
#endif
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    dependencyTree.resolve();
  }

  return srcFiles.size();
}
