#include "coddle.hpp"
#include "config.hpp"
#include "dependency_tree.hpp"
#include "error.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "library.hpp"
#include "osal.hpp"
#include "path_relative.hpp"
#include "perf.hpp"
#include "repository.hpp"
#include "resolver.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using IncToLib = std::unordered_map<std::string, std::vector<const Library *>>;

static std::string makeLibsFile(const std::string &fileName, const Config &config)
{
  return config.artifactsDir + "/" + fileName + ".libs";
}

// parses the file, derivies libraries from the file and saves result to
// the config.artifactsDir directory
static void srcToLibs(const std::string &fileName, const IncToLib &incToLib, const Config &config)
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

Coddle::Coddle()
{
  nullstrm.setstate(std::ios_base::badbit);
}

// generates *.libs file for each source and header file and generates
// [target].libs file
std::unordered_set<std::string> Coddle::generateLibsFiles(const Config &config) const
{
  Perf perf(__func__);
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

void Coddle::generateProjectLibsFile(const Config &config) const
{
  Perf perf(__func__);
  debug() << __func__ << "() generate libs file for whole project\n";
  std::string libsStr = [&]() {
    std::ostringstream libsStrm;
    debug() << __func__ << "() global libs:\n";
    for (auto &&lib : globalLibs)
    {
      debug() << __func__ << "() " << lib.first << " headers only: " << lib.second << std::endl;
      if (!lib.second)
        libsStrm << lib.first << std::endl;
    }
    debug() << __func__ << "() global pkgs:\n";
    for (auto &&pkg : pkgs)
    {
      debug() << __func__ << "() " << pkg << std::endl;
      libsStrm << pkg << std::endl;
    }
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

std::ostream &Coddle::debug() const
{
  if (verbose)
    return std::clog;
  else
    return nullstrm;
}

bool Coddle::downloadAndBuildLibs(const Config &config,
                                  const std::unordered_set<std::string> &localLibs)
{
  Perf perf(__func__);
  auto hasNativeLibs{false};
  debug() << __func__ << "() list of local libs:" << std::endl;
  for (auto &&libName : localLibs)
  {
    debug() << __func__ << "() " << libName << std::endl;
    auto it = repository.libraries.find(libName);
    if (it == std::end(repository.libraries))
      throw std::runtime_error("Library is not found: " + libName);
    auto &&lib = it->second;
    auto repoDir = ".coddle/libs_src/" + lib.name;
    switch (lib.type)
    {
    case Library::Type::File:
      globalLibsPushBack(std::make_pair(lib.name, false));
      break;
    case Library::Type::Git:
      globalLibsPushBack(std::make_pair(lib.name, false));
      if (!isDirExist(repoDir))
      {
        makeDir(".coddle/libs_src");
        execShowCmd("git clone --depth 1", lib.path, "-b", lib.version, repoDir);
        if (!lib.postClone.empty())
          execShowCmd("cd", repoDir, "&&", lib.postClone);
      }
      break;
    case Library::Type::PkgConfig: pkgs.insert(lib.name); break;
    case Library::Type::Lib: globalLibsPushBack(std::make_pair(lib.name, false)); break;
    case Library::Type::Framework: globalLibsPushBack({lib.name, false}); break;
    }
  }

  for (auto &&libName : localLibs)
  {
    auto it = repository.libraries.find(libName);
    if (it == std::end(repository.libraries))
      throw std::runtime_error("Library is not found: " + libName);
    auto &&lib = it->second;
    auto repoDir = ".coddle/libs_src/" + lib.name;
    if (lib.name != config.target &&
        (lib.type == Library::Type::File || lib.type == Library::Type::Git))
    {
      hasNativeLibs = true;
      auto libConfig = config;
      libConfig.shared = false;
      if (lib.type != Library::Type::File)
        libConfig.srcDir = repoDir;
      else
        libConfig.srcDir = lib.path;
      makeDir(".coddle/a");
      libConfig.targetDir = ".coddle/a";
      makeDir(".coddle/libs_artifacts/" + lib.name);
      libConfig.artifactsDir = ".coddle/libs_artifacts/" + lib.name;
      libConfig.target = lib.name;
      debug() << __func__ << "() building: " << lib.name << std::endl;
      const auto headersOnly = !build(libConfig);
      debug() << __func__ << "() " << lib.name << " headers only: " << headersOnly << std::endl;
      globalLibsPushBack(std::make_pair(lib.name, headersOnly));
    }
    if (lib.type == Library::Type::Lib)
      for (const auto &dep : lib.dependencies)
        globalLibsPushBack(std::make_pair(dep, false));
  }
  debug() << __func__ << "() hasNativeLibs: " << hasNativeLibs << std::endl;
  return hasNativeLibs;
}

bool Coddle::build(const Config &config)
{
  Perf perf(__func__);
  verbose = config.verbose;
  debug() << __func__ << "() started\n";
  std::unordered_set<std::string> localLibs = generateLibsFiles(config);
  auto hasNativeLibs = downloadAndBuildLibs(config, localLibs);
  generateProjectLibsFile(config);

  {
    debug() << __func__ << "() generate cflags files\n";
    auto cflagsStr = [&]() {
      std::ostringstream cflags;
      debug() << __func__ << "() packages\n";
      if (!pkgs.empty())
      {
        cflags << " $(pkg-config --cflags";
        for (const auto &pkg : pkgs)
          cflags << " " << pkg;
        cflags << ")";
      }

      debug() << __func__ << "() global libs:\n";
      for (auto &&libName : globalLibs)
      {
        debug() << __func__ << "() " << libName.first << " headers only: " << libName.second
                << std::endl;
        auto it = repository.libraries.find(libName.first);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName.first);
        auto &&lib = it->second;
        if (lib.type != Library::Type::File)
        {
          if (!lib.incdir.empty())
            cflags << " -I.coddle/libs_src/" << libName.first << "/" << lib.incdir;
        }
        else
        {
          cflags << " -I" << lib.path << "/";
          if (!lib.incdir.empty())
            cflags << lib.incdir;
          else
            cflags << "..";
        }
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

  debug() << __func__ << "() get list of source files\n";
  const std::vector<std::string> srcFiles = [&]() {
    std::vector<std::string> srcFiles;
    for (auto &&fileName : getFilesList(config.srcDir))
    {
      auto ext = getFileExtention(fileName);
      static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
      auto &&extention = getFileExtention(fileName);
      if (srcExtentions.find(extention) != std::end(srcExtentions))
        srcFiles.push_back(fileName);
    }
    return srcFiles;
  }();

  {
    DependencyTree dependencyTree;
    for (auto &&fileName : srcFiles)
    {
      {
        debug() << __func__ << "() compile source file: " << fileName << std::endl;
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
        dependency->exec = [fileName, &config, hasNativeLibs, this]() {
          std::ostringstream cmd;

          cmd << "clang++";
          {
            debug() << __func__ << "() load cflags\n";
            std::ifstream cflags(config.artifactsDir + "/cflags");
            if (cflags)
              cmd << cflags.rdbuf();
          }
          debug() << __func__ << "() include dirs\n";
          if (hasNativeLibs)
            cmd << " -I.coddle/libs_src";

          cmd << " -c " << config.srcDir << "/" << fileName << " -o " << config.artifactsDir << "/"
              << fileName + ".o";
          std::cout << cmd.str() << std::endl;
          cmd << " -MT " << config.srcDir << "/" << fileName << " -MMD -MF " << config.artifactsDir
              << "/" << fileName << ".mk";
#ifdef _WIN32
          cmd << " -Xclang -flto-visibility-public-std";
          if (config.winmain)
            cmd << " -Wl,-subsystem:windows";
#endif
          try
          {
            ::exec(cmd.str());
            {
              debug() << __func__ << "() generate .headers\n";
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

      {
        debug() << __func__ << "() determine is the project executable or library\n";
        // TODO: use better heuristic
        auto &&dependency =
          dependencyTree.addTarget(config.artifactsDir + "/" + fileName + ".hasmain");
        dependency->dependsOf(fileName);
        dependency->exec = [fileName, &config, this]() {
          std::ostringstream cmd;
          auto hasMain = false;
          {
            debug() << __func__ << "() parse source file: " << fileName << std::endl;
            std::ifstream srcFile(config.srcDir + "/" + fileName);
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

  {
    debug() << __func__ << "() linking\n";
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
    {
      std::ofstream objFiles(config.artifactsDir + "/" + config.target + ".objs");
      for (auto &&fileName : srcFiles)
        objFiles << " " << config.artifactsDir << "/" << fileName << ".o";
    }
    DependencyTree dependencyTree;
    if (isExec || config.shared)
    {

#ifdef _WIN32
      const auto target = config.targetDir + "/" + config.target + (config.shared ? ".dll" : ".exe");
#else
      const auto target = config.targetDir + "/" + config.target + (config.shared ? ".so" : "");
#endif
      auto &&dependency = dependencyTree.addTarget(target);
      std::ostringstream strm;
      strm << "clang++";
      if (config.shared)
        strm << " -shared";
      strm << "$(cat " << config.artifactsDir << "/" << config.target << ".objs)";
      for (auto &&fileName : srcFiles)
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      strm << " -o " << target //
           << " -L/usr/lib -L/usr/local/lib";
      if (hasNativeLibs)
        strm << " -L.coddle/a";

      for (auto &&libName : globalLibs)
      {
        if (libName.second)
        {
          debug() << __func__ << "() " << libName.first << " is headers only library\n";
          continue;
        }
        auto it = repository.libraries.find(libName.first);
        if (it == std::end(repository.libraries))
          throw std::runtime_error("Library is not found: " + libName.first);
        auto &&lib = it->second;
        switch (lib.type)
        {
        case Library::Type::File:
        case Library::Type::Git:
        case Library::Type::Lib: strm << " -l" << lib.name; break;
        case Library::Type::Framework: strm << " -framework " << lib.name; break;
        default:
          THROW_ERROR("Unknwon lib type " << toString(lib.type) << " of library " << lib.name);
        }
        if (lib.type == Library::Type::File || lib.type == Library::Type::Git)
#ifndef _WIN32
          dependency->dependsOf(".coddle/a/lib" + lib.name + ".a");
#else
          dependency->dependsOf(".coddle/a/" + lib.name + ".lib");
#endif
        if (!lib.libdir.empty())
          strm << " -L.coddle/libs_src/" << libName.first << "/" << lib.libdir;
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
      if (config.winmain)
        strm << " -Wl,-subsystem:windows";
#endif

      // TODO ldflags
      if (config.multithreaded)
        // FIXME: trigger relink if changed
        strm << " -pthread";
      dependency->dependsOf(config.artifactsDir + "/ldflags");
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    else if (!srcFiles.empty() && !config.shared)
    {
const auto target = config.targetDir + "/lib" + config.target + ".a";
      auto &&dependency =
        dependencyTree.addTarget(target);
      for (auto &&fileName : srcFiles)
        dependency->dependsOf(config.artifactsDir + "/" + fileName + ".o");
      std::ostringstream strm;
      strm << "ar r " << target //
           << "$(cat " << config.artifactsDir << "/" << config.target << ".objs)";
      auto cmd = strm.str();
      dependency->exec = [cmd]() {
        std::cout << cmd << std::endl;
        ::exec(cmd);
      };
    }
    dependencyTree.resolve();
  }

  debug() << __func__ << "() source files: " << srcFiles.size() << std::endl;
  debug() << __func__ << "() ended\n";

  return srcFiles.size() > 0;
}

std::vector<std::pair<std::string, bool>>::const_iterator Coddle::globalLibsFind(
  const std::string &value) const
{
  return std::find_if(std::begin(globalLibs),
                      std::end(globalLibs),
                      [&value](const std::pair<std::string, bool> &x) { return x.first == value; });
}

std::vector<std::pair<std::string, bool>>::iterator Coddle::globalLibsFind(const std::string &value)
{
  return std::find_if(std::begin(globalLibs),
                      std::end(globalLibs),
                      [&value](const std::pair<std::string, bool> &x) { return x.first == value; });
}

void Coddle::globalLibsPushBack(const std::pair<std::string, bool> &value)
{
  debug() << __func__ << "() lib: " << value.first << " isHeadersOnly: " << value.second
          << std::endl;
  auto it = globalLibsFind(value.first);
  if (it == std::end(globalLibs))
    globalLibs.push_back(value);
  else
    it->second = value.second;
}
