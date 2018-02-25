#include "config.hpp"
#ifndef _WIN32
#include "gcc_driver.hpp"
#else
#include "vs_driver.hpp"
#endif
#include "error.hpp"
#include "file_exist.hpp"
#include "file_extention.hpp"
#include "file_name.hpp"
#include "make_path.hpp"
#include "osal.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

static void pullGitLib(const std::string &url, const std::string &version, std::string dirName = "")
{
  makeDir(makePath(".coddle", "libs"));
  changeDir(makePath(".coddle", "libs"));
  if (dirName.empty())
  {
    auto p = url.rfind("/");
    dirName = url.substr(p + 1);
    dirName.resize(dirName.size() - 4);
  }
  if (isDirExist(dirName))
  {
    changeDir("../..");
    return;
  }
  execShowCmd("git clone --depth 1", url, "-b", version, dirName);
  changeDir("../..");
}

static void getLib(Config &config, const std::string &lib, const std::string &version)
{
  setenv(
    "PKG_CONFIG_PATH", makePath(".coddle", "libs", "usr", "local", "lib", "pkgconfig").c_str(), 1);
  static std::unordered_map<std::string,
                            std::function<void(Config & config, const std::string &version)>>
    libs = {{"jsoncpp",
             [](Config &config, const std::string &version) {
               if (!isFileExist(
                     makePath(".coddle", "libs", "usr", "local", "lib", "pkgconfig", "jsoncpp.pc")))
               {
                 pullGitLib("https://github.com/open-source-parsers/jsoncpp.git", version);
                 auto cwd = getCurrentWorkingDir();
                 auto path = makePath(".coddle", "libs", "jsoncpp");
                 std::cout << "coddle: Entering directory `.coddle/libs/jsoncpp'\n";
                 execShowCmd(
                   "cd",
                   path,
                   "&& cmake -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON "
                   "-DCMAKE_INSTALL_PREFIX:PATH=" +
                     makePath(cwd, ".coddle", "libs", "usr", "local"),
                   ". && make -j",
                   config.njobs,
                   "&& make install");
                 std::cout << "coddle: Leaving directory `.coddle/libs/jsoncpp'\n";
               }
               config.incToPkg["json/json.h"].push_back("jsoncpp");
             }},
            {"mongoose",
             [](Config &config, const std::string &version) {
               pullGitLib("https://github.com/cesanta/mongoose.git", version);
               ProjectConfig cfg;
               cfg.srcDirs.push_back(".coddle/libs/mongoose");
               cfg.targetType = TargetType::StaticLib;
               config.projects.push_back(cfg);
               config.common.incDirs.push_back(".coddle/libs/mongoose");
             }},
            {"curl",
             [](Config &config, const std::string &version) {
               if (!isFileExist(
                     makePath(".coddle", "libs", "usr", "local", "lib", "pkgconfig", "libcurl.pc")))
               {
                 pullGitLib("https://github.com/curl/curl.git", version);
                 auto cwd = getCurrentWorkingDir();
                 auto path = makePath(".coddle", "libs", "curl");
                 std::cout << "coddle: Entering directory `.coddle/libs/curl'\n";
                 execShowCmd("cd",
                             path,
                             "&& ./buildconf && ./configure --prefix=" +
                               makePath(cwd, ".coddle", "libs", "usr", "local"),
#ifdef __APPLE__
                             "--with-darwinssl",
#endif
                             "&& make -j",
                             config.njobs,
                             "&& make install");
                 std::cout << "coddle: Leaving directory `.coddle/libs/curl'\n";
               }
               config.incToPkg["curl/curl.h"].push_back("libcurl");
             }},
            {"sdl2",
             [](Config &config, const std::string &version) {
               if (!isFileExist(
                     makePath(".coddle", "libs", "usr", "local", "lib", "pkgconfig", "sdl2.pc")))
               {
                 pullGitLib("https://github.com/spurious/SDL-mirror.git", version, "sdl2");
                 auto cwd = getCurrentWorkingDir();
                 auto path = makePath(".coddle", "libs", "sdl2");
                 std::cout << "coddle: Entering directory `.coddle/libs/sdl2'\n";
                 execShowCmd("cd",
                             path,
                             "&& ./configure --prefix=" +
                               makePath(cwd, ".coddle", "libs", "usr", "local"),
                             "&& make -j",
                             config.njobs,
                             "&& make install");
                 std::cout << "coddle: Leaving directory `.coddle/libs/sdl2'\n";
               }
               config.incToPkg.erase("SDL.h");
               config.incToPkg["SDL.h"].push_back("sdl2");
             }},
            {"sdlpp", [](Config &config, const std::string &version) {
               pullGitLib("https://github.com/antonte/sdlpp.git", version);
               config.common.incDirs.push_back(".coddle/libs/sdlpp");
             }}};
  auto iter = libs.find(lib);
  if (iter == std::end(libs))
    THROW_ERROR("Library " << lib << " not found");
  return iter->second(config, version);
}

static void addProject(Config &cfg, const std::string &dir, TargetType type)
{
  ProjectConfig prj;
  prj.srcDirs.push_back(dir);
  prj.targetType = type;
  cfg.projects.push_back(prj);
}

Config::Config(int argc, char **argv)
  :
#ifndef _WIN32
    driver{std::make_shared<GccDriver>()},
#else
    driver{std::make_shared<VsDriver>()},
#endif
    isDirExist(&::isDirExist),
    isFileExist(&::isFileExist),
    fileName(&::fileName),
    getCurrentWorkingDir(&::getCurrentWorkingDir),
    getExecPath(&::getExecPath),
    getFileExtention(&::getFileExtention),
    getFilesList(&::getFilesList),
    getFileModification(&::getFileModification),
    changeDir(&::changeDir),
    exec(&::exec),
    execShowCmd(&::execShowCmd),
    pullGitLib(&::pullGitLib),
    getLib(&::getLib),
    addProject(&::addProject)
{
  for (auto i = 0; i < argc; ++i)
    args.push_back(argv[0]);
#ifndef _WIN32
  common.cflags.push_back("-Wall");
  common.cflags.push_back("-Wextra");
  common.cflags.push_back("-march=native");
  common.cflags.push_back("-gdwarf-3");
  common.cflags.push_back("-O3");
  common.cflags.push_back("-g");
#else
  common.cflags.push_back("/EHsc");
  common.cflags.push_back("/W4");
  common.cflags.push_back("/Ox");
  common.cflags.push_back("/nologo");
  common.ldflags.push_back("/nologo");
#endif
  incToPkg["SDL.h"].push_back("sdl2");
  incToPkg["SDL_ttf.h"].push_back("SDL2_ttf");
  incToPkg["SDL_image.h"].push_back("SDL2_image");
  incToPkg["fftw3.h"].push_back("fftw3");
  incToPkg["libswscale/swscale.h"].push_back("libswscale");
  {
    auto &inc = incToPkg["libavformat/avformat.h"];
    inc.insert(std::end(inc), {"libavformat", "libavutil", "libavcodec"});
  }
  {
    auto &inc = incToPkg["libavdevice/avdevice.h"];
    inc.insert(std::end(inc), {"libavdevice", "libavutil", "libavcodec"});
  }
  incToPkg["curl/curl.h"].push_back("libcurl");
  incToPkg["pulse/thread-mainloop.h"].push_back("libpulse");
  incToPkg["pulse/context.h"].push_back("libpulse");
  incToPkg["pulse/stream.h"].push_back("libpulse");
  incToPkg["pulse/scache.h"].push_back("libpulse");

  incToLib["GL/glut.h"].push_back("glut");
  incToLib["GL/glut.h"].push_back("GL");
}

std::vector<std::string> Config::merge(const std::vector<std::string> &x,
                                       const std::vector<std::string> &y)
{
  std::unordered_set<std::string> cache;
  std::vector<std::string> res;
  for (const auto &i : x)
  {
    if (cache.find(i) == std::end(cache))
      res.push_back(i);
    cache.insert(i);
  }
  for (const auto &i : y)
  {
    if (cache.find(i) == std::end(cache))
      res.push_back(i);
    cache.insert(i);
  }
  return res;
}
