#include "config.hpp"
#include "error.hpp"
#include "file_extention.hpp"
#include "file_name.hpp"
#include "func.hpp"
#include "osal.hpp"
#include "perf.hpp"
#include "repository.hpp"
#include "thread_pool.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>

static bool verbose = false;

struct LibRet
{
  LibRet(const std::string &name = {}, bool headersOnly = false) : name(name), headersOnly(headersOnly) {}
  std::string name;
  bool headersOnly{false};
  bool operator==(const LibRet &other) const { return name == other.name; }
#define SER_PROPERTY_LIST \
  SER_PROPERTY(name);     \
  SER_PROPERTY(headersOnly);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

namespace std
{
  template <>
  struct hash<LibRet>
  {
    std::size_t operator()(const LibRet &k) const { return hash<std::string>()(k.name); }
  };

} // namespace std

struct BuildRet
{
  std::vector<LibRet> libs;
  File binary;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(libs);     \
  SER_PROPERTY(binary);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

BuildRet build(const Config &cfg, const Repository &repo);

void pushBack(std::vector<LibRet> &x, const std::vector<LibRet> &y)
{
  for (const auto &i : y)
    if (std::find_if(std::begin(x),
                     std::end(x), //
                     [&i](const LibRet &j) { return i.name == j.name; }) == std::end(x))
      x.push_back(i);
}

void pushBack(std::vector<std::string> &x, const std::vector<std::string> &y)
{
  for (const auto &i : y)
    if (std::find(std::begin(x), std::end(x), i) == std::end(x))
      x.push_back(i);
}

std::vector<LibRet> buildLib(const std::string &libName, const Repository &repo, bool debug)
{
  auto it = repo.libraries.find(libName);
  if (it == std::end(repo.libraries))
    throw std::runtime_error("Library is not found: " + libName);
  auto &&lib = it->second;

  if (lib.type == Library::Type::Git)
  {
    const auto repoDir = ".coddle/libs_src/" + lib.name;

    if (!isDirExist(repoDir))
    {
      makeDir(".coddle/libs_src");
      execShowCmd("git clone --depth 1", lib.path, "-b", lib.version, repoDir);
      if (!lib.postClone.empty())
      {
        {
          std::ofstream sh(repoDir + "/postClone.sh");
          sh << "#!/bin/bash\n" //
             << lib.postClone;
        }
        execShowCmd("bash -c \"cd", repoDir, "&& ./postClone.sh\"");
      }
    }
    makeDir(".coddle/a");
    makeDir(".coddle/libs_artifacts/" + lib.name);
    Config cfg;
    cfg.target = lib.name;
    cfg.srcDir = repoDir;
    cfg.targetDir = ".coddle/a";
    cfg.artifactsDir = ".coddle/libs_artifacts/" + lib.name;
    cfg.debug = debug;
    cfg.shared = false;
    return func(build, cfg, repo).libs;
  }
  else if (lib.type == Library::Type::File)
  {
    makeDir(".coddle/a");
    makeDir(".coddle/libs_artifacts/" + lib.name);
    Config cfg;
    cfg.target = lib.name;
    cfg.srcDir = lib.path;
    cfg.targetDir = ".coddle/a";
    cfg.artifactsDir = ".coddle/libs_artifacts/" + lib.name;
    cfg.debug = debug;
    cfg.shared = false;
    return build(cfg, repo).libs;
  }
  std::vector<LibRet> ret = {LibRet{lib.name, false}};
  {
    ThreadPool thPool;

    for (const auto &dep : lib.dependencies)
    {
      auto buildLibRet = std::make_shared<decltype(buildLib(dep, repo, debug))>();
      thPool.addJob([buildLibRet, dep, &repo, &debug]() { *buildLibRet = buildLib(dep, repo, debug); },
                    [buildLibRet, &ret]() { pushBack(ret, *buildLibRet); });
    }
    for (const auto &dep : lib.dependencies)
    {
      (void)dep;
      thPool.waitForOne();
    }
  }
  std::sort(std::begin(ret), std::end(ret), [](const LibRet &x, const LibRet &y) { return x.name < y.name; });
  return ret;
}

std::vector<std::string> getLibsFromFile(const File &file, const Repository &repo, bool /*debug*/)
{
  std::ifstream srcFile(file.name);
  std::string line;
  std::unordered_set<std::string> headerList;
  while (std::getline(srcFile, line))
  {
    line = [](const std::string &x) {
      std::string res;
      for (const auto ch : x)
      {
        if (ch <= ' ' && ch >= 0)
          continue;
        if (ch == '\\')
          res += '/';
        else
          res += ch;
      }
      return res;
    }(line);
    if (line.find("#include") == 0)
    {
      const auto p = line.find_last_of("\">");
      if (p == line.size() - 1)
      {
        auto header = line.substr(9);
        header.resize(header.size() - 1);
        headerList.insert(header);
      }
    }

    if (line.find("#include<") != 0)
      continue;
    const auto p = line.find(">");
    if (p != line.size() - 1)
      continue;
    auto header = line.substr(9);
    header.resize(header.size() - 1);
    headerList.insert(header);
  }
  std::unordered_set<std::string> ret;
  for (const auto &header : headerList)
  {
    const auto it = repo.incToLib.find(header);
    if (it == std::end(repo.incToLib))
      continue;
    for (const auto &lib : it->second)
      ret.insert(lib->name);
  }
  return {std::begin(ret), std::end(ret)};
}

std::vector<LibRet> getLibsFromFiles(const std::string &currentTarget, const std::vector<File> &files, const Repository &repo, bool debug)
{
  std::vector<std::string> libs;
  {
    ThreadPool thPool;
    for (const auto &file : files)
    {
      auto funcRet = std::make_shared<decltype(getLibsFromFile(file, repo, debug))>();
      thPool.addJob([funcRet, file, &repo, &debug]() { *funcRet = func(getLibsFromFile, file, repo, debug); },
                    [funcRet, &libs]() { pushBack(libs, *funcRet); });
    }
    for (const auto &file : files)
    {
      (void)file;
      thPool.waitForOne();
    }
  }
  std::sort(std::begin(libs), std::end(libs));

  std::vector<LibRet> ret;
  for (const auto &lib : libs)
  {
    if (lib == currentTarget)
    {
      ret.emplace_back(lib, false);
      continue;
    }
    const auto tmp = buildLib(lib, repo, debug);
    ret.insert(std::end(ret), std::begin(tmp), std::end(tmp));
  }

  return {std::begin(ret), std::end(ret)};
}

struct CompileRet
{
  File obj;
  std::vector<File> headers;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(obj);      \
  SER_PROPERTY(headers);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

CompileRet compile(const File &file, const std::string &cflags, bool hasNativeLibs, const std::string artifactsDir, bool winmain)
{
  const auto fn = fileName(file.name);

  std::ostringstream cmd;
  cmd << "clang++";
  {
    cmd << cflags;
  }
  if (hasNativeLibs)
    cmd << " -I.coddle/libs_src";

  auto objFileName = artifactsDir + "/" + fn + ".o";

  cmd << " -c " << file.name << " -o " << objFileName;
  std::cout << cmd.str() << std::endl;
  cmd << " -MT " << file.name << " -MMD -MF " << artifactsDir << "/" << fn << ".mk";
#ifdef _WIN32
  cmd << " -Xclang -flto-visibility-public-std";
  if (winmain)
    cmd << " -Wl,-subsystem:windows";
#else
  (void)winmain;
#endif
  CompileRet ret;
  try
  {
    ::exec(cmd.str());
    ret.obj = File{objFileName};
    {
      std::string mk = [&fn, &artifactsDir]() {
        std::ifstream f(artifactsDir + "/" + fn + ".mk");
        std::ostringstream strm;
        f >> strm.rdbuf();
        return strm.str();
      }();
      remove((artifactsDir + "/" + fn + ".mk").c_str());
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
      while (std::getline(strm, header, ' '))
        if (!header.empty())
          ret.headers.push_back(File{header});
    }
  }
  catch (std::exception &e)
  {
    std::cout << "coddle: *** [" + file.name + "] " + e.what() << std::endl;
  }

  return ret;
}

struct LinkRet
{
  std::optional<LibRet> lib;
  File binary;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(lib);      \
  SER_PROPERTY(binary);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

LinkRet link_(const std::string &targetDir,
              const std::string &targetFile,
              const std::string &ldflags,
              bool isExec,
              bool shared,
              bool multithreaded,
              bool winmain,
              const std::string &artifactsDir,
              const std::vector<File> &objs,
              const std::vector<LibRet> &libs,
              const std::vector<File> &,
              const std::vector<std::string> &pkgs,
              const Repository &repo)
{
  if (objs.empty())
  {
    LinkRet ret;
    LibRet tmp;
    tmp.name = targetFile;
    tmp.headersOnly = true;
    ret.lib = tmp;
    return ret;
  }
  const auto objFiles = [&] {
    std::ostringstream ss;
    for (const auto &obj : objs)
      ss << " " << obj.name;
    const auto str = ss.str();
    if (str.size() > 2000)
    {
      const auto fileName = artifactsDir + "/" + targetFile + ".objs";
      auto fs = std::ofstream{fileName};
      fs << str;
      return "$(cat " + fileName;
    }
    return str;
  }();

  if (isExec || shared)
  {
#ifdef _WIN32
    const auto target = targetDir + "/" + targetFile + (shared ? ".dll" : ".exe");
#else
    const auto target = targetDir + "/" + targetFile + (shared ? ".so" : "");
#endif
    std::ostringstream strm;
    strm << "clang++";
    if (shared)
      strm << " -shared";
    strm << objFiles;

    strm << " -o " << target;
#if defined(_WIN32) || defined(__APPLE__)
    strm << " -L/usr/lib -L/usr/local/lib";
#endif
    if (!ldflags.empty())
      strm << " " << ldflags;
    for (const auto &libRet : libs)
    {
      auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      auto &&lib = it->second;
      if (lib.type == Library::Type::File || lib.type == Library::Type::Git)
      {
        strm << " -L.coddle/a";
        break;
      }
    }

    for (const auto &libRet : libs)
    {
      if (libRet.headersOnly)
        continue;
      if (libRet.name == targetFile)
        continue;
      auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      auto &&lib = it->second;
      switch (lib.type)
      {
      case Library::Type::File:
      case Library::Type::Git:
      case Library::Type::Lib: strm << " -l" << lib.name; break;
      case Library::Type::Framework: strm << " -framework " << lib.name; break;
      case Library::Type::PkgConfig: break;
      default: THROW_ERROR("Unknwon lib type " << toString(lib.type) << " of library " << lib.name);
      }
      if (lib.type == Library::Type::File || lib.type == Library::Type::Git)
        if (!lib.libdir.empty())
          strm << " -L.coddle/libs_src/" << lib.name << "/" << lib.libdir;
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
    if (winmain)
      strm << " -Wl,-subsystem:windows";
#else
    (void)winmain;
#endif

    // TODO ldflags
    if (multithreaded)
      strm << " -pthread";
    auto cmd = strm.str();
    std::cout << cmd << std::endl;
    ::exec(cmd);

    LinkRet ret;
    LibRet tmp;
    tmp.name = targetFile;
    tmp.headersOnly = false;
    ret.lib = tmp;
    ret.binary = File{target};
    return ret;
  }

  const auto target = targetDir + "/lib" + targetFile + ".a";
  std::ostringstream strm;
  strm << "ar r " << target << objFiles;
  auto cmd = strm.str();
  std::cout << cmd << std::endl;
  ::exec(cmd);
  LinkRet ret;
  LibRet tmp;
  tmp.name = targetFile;
  tmp.headersOnly = false;
  ret.lib = tmp;
  ret.binary = File{target};
  return ret;
}

bool fileHasMain(const File &file)
{
  {
    std::ifstream srcFile(file.name);
    std::string line;
    while (std::getline(srcFile, line))
      if (line.find("int main(") == 0)
        return true;
  }
  {
    std::ifstream srcFile(file.name);
    std::string line;
    while (std::getline(srcFile, line))
      if (line.find("auto main(") == 0)
        return true;
  }
  return false;
};

static bool isAbsDir(const std::string &dir)
{
  return std::filesystem::path(dir).is_absolute();
}

static std::string normalize(const std::string &path)
{
  return std::filesystem::path(path).lexically_normal().string();
}

BuildRet build(const Config &cfg, const Repository &repo)
{
  const std::vector<File> files = [&cfg]() {
    std::vector<File> ret;
    static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
    static std::unordered_set<std::string> headerExtentions = {"h", "hpp", "h++", "H"};
    for (const auto &fileName : getFilesList(cfg.srcDir))
    {
      const auto extention = getFileExtention(fileName);
      if (srcExtentions.find(extention) == std::end(srcExtentions) && headerExtentions.find(extention) == std::end(headerExtentions))
        continue;
      ret.emplace_back(cfg.srcDir + "/" + fileName);
    }
    std::sort(std::begin(ret), std::end(ret), [](const File &x, const File &y) { return x.name < y.name; });
    return ret;
  }();
  const auto libs = getLibsFromFiles(cfg.target, files, repo, cfg.debug);
  const auto pkgs = [&libs, &repo]() -> std::vector<std::string> {
    std::set<std::string> ret;
    for (const auto &libRet : libs)
    {
      const auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      const auto &lib = it->second;
      if (lib.type == Library::Type::PkgConfig)
        ret.insert(lib.name);
    }
    return {std::begin(ret), std::end(ret)};
  }();
  const auto cflags = [&libs, &pkgs, &cfg, &repo]() {
    std::ostringstream cflags;
    if (!pkgs.empty())
    {
      cflags << " $(pkg-config --cflags";
      for (const auto &pkg : pkgs)
        cflags << " " << pkg;
      cflags << ")";
    }

    std::set<std::string> incDirs;

    for (const auto &libRet : libs)
    {
      const auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      const auto &lib = it->second;
      if (lib.type != Library::Type::File)
      {
        if (!lib.incdir.empty())
        {
          if (!isAbsDir(lib.incdir))
            incDirs.insert(normalize(".coddle/libs_src/" + libRet.name + "/" + lib.incdir));
          else
            incDirs.insert(normalize(lib.incdir));
        }
        for (const auto &incdir : lib.incdirs)
        {
          if (!isAbsDir(incdir))
            incDirs.insert(normalize(".coddle/libs_src/" + libRet.name + "/" + incdir));
          else
            incDirs.insert(normalize(incdir));
        }
      }
      else
      {
        if (lib.incdir.empty() && lib.incdirs.empty())
          incDirs.insert(normalize(lib.path + "/.."));
        else
        {
          if (!isAbsDir(lib.incdir))
            incDirs.insert(normalize(lib.path + "/" + lib.incdir));
          else
            incDirs.insert(normalize(lib.incdir));
          for (const auto &incdir : lib.incdirs)
          {
            if (!isAbsDir(incdir))
              incDirs.insert(normalize(lib.path + "/" + incdir));
            else
              incDirs.insert(normalize(incdir));
          }
        }
      }
    }
    for (const auto &inc : incDirs)
      cflags << " -I" << inc;

    cflags << " " << cfg.cflags;
    if (cfg.debug)
      cflags << " -g -O0 -D_GLIBCXX_DEBUG";
    else
      cflags << " -O3 -march=native";
    if (cfg.multithreaded)
      cflags << " -pthread";
    return cflags.str();
  }();
  const auto hasNativeLibs = [&libs, &repo]() {
    for (auto &&libRet : libs)
    {
      auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      auto &&lib = it->second;
      if (lib.type == Library::Type::File)
        return true;
      if (lib.type == Library::Type::Git)
        return true;
    }
    return false;
  }();
  const auto srcFiles = [&files]() {
    std::vector<File> ret;
    for (const auto &file : files)
    {
      const auto ext = getFileExtention(file.name);
      static std::unordered_set<std::string> srcExtentions = {"c", "cpp", "c++", "C"};
      const auto extention = getFileExtention(file.name);
      if (srcExtentions.find(extention) == std::end(srcExtentions))
        continue;
      ret.push_back(file);
    }
    return ret;
  }();

  const auto objs = [&srcFiles, &cfg, hasNativeLibs, &cflags]() {
    std::vector<File> ret;
    {
      ThreadPool thPool;

      for (const auto &file : srcFiles)
      {
        auto funcRet = std::make_shared<decltype(func(compile, file, cflags, hasNativeLibs, cfg.artifactsDir, cfg.winmain))>();
        thPool.addJob(
          [funcRet, file, &cflags, &hasNativeLibs, &cfg]() { *funcRet = func(compile, file, cflags, hasNativeLibs, cfg.artifactsDir, cfg.winmain); },
          [funcRet, &ret]() { ret.emplace_back(funcRet->obj); });
      }
      for (const auto &file : srcFiles)
      {
        (void)file;
        thPool.waitForOne();
      }
    }
    std::sort(std::begin(ret), std::end(ret), [](const File &x, const File &y) { return x.name < y.name; });
    return ret;
  }();
  auto hasMain = [&srcFiles]() {
    for (const auto &srcFile : srcFiles)
      if (func(fileHasMain, srcFile))
        return true;
    return false;
  }();

  const auto fileLibs = [&libs, &repo]() {
    std::vector<File> ret;
    for (const auto &libRet : libs)
    {
      const auto it = repo.libraries.find(libRet.name);
      if (it == std::end(repo.libraries))
        throw std::runtime_error("Library is not found: " + libRet.name);
      const auto &lib = it->second;
      if ((lib.type == Library::Type::File || lib.type == Library::Type::Git) && !libRet.headersOnly)
      {
        File tmp(".coddle/a/lib" + lib.name + ".a");
        ret.push_back(tmp);
      }
    }
    return ret;
  }();

  auto linkRet = func(link_,
                      cfg.targetDir,
                      cfg.target,
                      cfg.ldflags,
                      hasMain,
                      cfg.shared,
                      cfg.multithreaded,
                      cfg.winmain,
                      cfg.artifactsDir,
                      objs,
                      libs,
                      fileLibs,
                      pkgs,
                      repo);

  if (!linkRet.lib)
  {
    BuildRet ret;
    ret.libs = libs;
    ret.binary = linkRet.binary;
    return ret;
  }
  BuildRet ret;
  ret.libs = {*linkRet.lib};
  pushBack(ret.libs, libs);
  ret.binary = linkRet.binary;
  return ret;
}

int main(int argc, char **argv)
{
  try
  {
    Config config(argc, argv);

    Repository repo(config.localRepository, config.remoteRepository, config.remoteVersion);
    verbose = config.verbose;
    build(config, repo);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch (int e)
  {
    return e;
  }
}
