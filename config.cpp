#include "config.hpp"
#ifndef _WIN32
#include "gcc_driver.hpp"
#else
#include "vs_driver.hpp"
#endif
#include "file_exist.hpp"
#include "file_name.hpp"
#include "file_extention.hpp"
#include "osal.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

static void pullGitLib(const std::string &url, const std::string &version)
{
  makeDir(".gitlibs");
  changeDir(".gitlibs");
  auto p = url.rfind("/");
  auto dirName = url.substr(p + 1);
  dirName.resize(dirName.size() - 4);
  if (isDirExist(dirName))
  {
    changeDir("..");
    return;
  }
  execShowCmd("git clone --depth 1", url, "-b", version, dirName);
  changeDir("..");
}

Config::Config(int argc, char **argv):
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
  pullGitLib(&::pullGitLib)
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
  incToPkg["fftw3.h"].push_back("fftw3");
  incToPkg["libswscale/swscale.h"].push_back("libswscale");
  {
    auto &inc = incToPkg["libavformat/avformat.h"];
    inc.insert(std::end(inc), { "libavformat", "libavutil", "libavcodec" });
  }
  {
    auto &inc = incToPkg["libavdevice/avdevice.h"];
    inc.insert(std::end(inc), { "libavdevice", "libavutil", "libavcodec" });
  }
  incToPkg["pulse/thread-mainloop.h"].push_back("libpulse");
  incToPkg["pulse/context.h"].push_back("libpulse");
  incToPkg["pulse/stream.h"].push_back("libpulse");
  incToPkg["pulse/scache.h"].push_back("libpulse");

  incToLib["GL/glut.h"].push_back("glut");
  incToLib["GL/glut.h"].push_back("GL");
}

std::vector<std::string> Config::merge(const std::vector<std::string> &x, const std::vector<std::string> &y)
{
  std::vector<std::string> res = x;
  std::unordered_set<std::string> cache{std::begin(x), std::end(x)};
  for (const auto &i: y)
  {
    if (cache.find(i) == std::end(cache))
      res.push_back(i);
    cache.insert(i);
  }
  return res;
}
