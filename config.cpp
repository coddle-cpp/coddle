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
#include "osal.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

static void pullGitLib(const std::string &url, const std::string &version, std::string dirName = "")
{
  makeDir(".coddle/libs");
  changeDir(".coddle/libs");
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
  static std::unordered_map<std::string, std::function<void(Config &config, const std::string &version)>> libs = {
    {
      "jsoncpp", [](Config &config, const std::string &version)
      {
        pullGitLib("https://github.com/open-source-parsers/jsoncpp.git", version);
        ProjectConfig cfg;
        cfg.srcDirs.push_back(".coddle/libs/jsoncpp");
        cfg.targetType = TargetType::StaticLib;
        cfg.srcDirs.push_back(".coddle/libs/jsoncpp/src/lib_json");
        cfg.cflags.push_back("-Wconversion");
        cfg.cflags.push_back("-Wshadow");
        cfg.cflags.push_back("-Wextra");
        cfg.cflags.push_back("-pedantic");
        cfg.cflags.push_back("-Werror=strict-aliasing");
        cfg.cflags.push_back("-DNDEBUG");
        config.projects.push_back(cfg);
        config.common.cflags.push_back("-I.coddle/libs/jsoncpp/include");
      }
    },
    {
      "mongoose", [](Config &config, const std::string &version)
      {
        pullGitLib("https://github.com/cesanta/mongoose.git", version);
        ProjectConfig cfg;
        cfg.srcDirs.push_back(".coddle/libs/mongoose");
        cfg.targetType = TargetType::StaticLib;
        config.projects.push_back(cfg);
        config.common.cflags.push_back("-I.coddle/libs/mongoose");
      }
    },
    {
      "curl", [](Config &config, const std::string &version)
      {
        pullGitLib("https://github.com/curl/curl.git", version);
        std::cout << "coddle: Entering directory `.coddle/libs/curl'\n";
        exec("cd .coddle/libs/curl && if [ ! -f configure ]; then ./buildconf && ./configure; fi");
        std::cout << "coddle: Leaving directory `.coddle/libs/curl'\n";
        ProjectConfig cfg;
        cfg.srcDirs.push_back(".coddle/libs/curl/lib");
        cfg.srcDirs.push_back(".coddle/libs/curl/lib/vtls");
        cfg.srcDirs.push_back(".coddle/libs/curl/lib/vauth");
        cfg.target = ".coddle/libs/curl/libcurl.a";
        cfg.targetType = TargetType::StaticLib;
        cfg.cflags.push_back("-DHAVE_CONFIG_H");
        cfg.cflags.push_back("-I.coddle/libs/curl/include/curl");
        cfg.cflags.push_back("-I.coddle/libs/curl/lib");
        cfg.cflags.push_back("-DBUILDING_LIBCURL");
        cfg.cflags.push_back("-DCURL_HIDDEN_SYMBOLS");
        cfg.cflags.push_back("-fvisibility=hidden");
        cfg.cflags.push_back("-Wno-system-headers");
        cfg.libs.push_back("z");
        config.projects.push_back(cfg);
        config.common.cflags.push_back("-I.coddle/libs/curl/include");
      }
    },
    {
      "sdl2", [](Config &config, const std::string &version)
      {
        pullGitLib("https://github.com/spurious/SDL-mirror.git", version, "sdl2");
        std::cout << "coddle: Entering directory `.coddle/libs/sdl2'\n";
        exec("cd .coddle/libs/sdl2 && if [ ! -f Makefile ]; then ./configure; fi");
        exec("cd .coddle/libs/sdl2 && rm -f src/render/direct3d11/SDL_render_winrt.cpp");
        std::cout << "coddle: Leaving directory `.coddle/libs/sdl2'\n";
        ProjectConfig cfg;
        cfg.targetType = TargetType::StaticLib;
        cfg.target = ".coddle/libs/sdl2/libsdl2.a";
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/atomic");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/cpuinfo");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/dynapi");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/events");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/file");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/haptic");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/joystick");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/libm");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/power");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/direct3d");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/direct3d11");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/opengl");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/opengles");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/opengles2");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/sndio");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/psp");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/render/software");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/stdlib");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/thread");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/timer");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/video");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/video/dummy");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/disk");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/dummy");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/loadso/dlopen");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/dsp");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/alsa");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/pulseaudio");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/audio/sndio");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/video/x11");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/core/linux");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/thread/pthread");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/joystick/linux");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/haptic/linux");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/power/linux");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/filesystem/unix");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/timer/unix");
        cfg.srcDirs.push_back(".coddle/libs/sdl2/src/core/linux");
        cfg.cflags.push_back("-DUSING_GENERATED_CONFIG_H");
        cfg.cflags.push_back("-mmmx");
        cfg.cflags.push_back("-m3dnow");
        cfg.cflags.push_back("-msse");
        cfg.cflags.push_back("-msse2");
        cfg.cflags.push_back("-msse3");
        cfg.cflags.push_back("-fvisibility=hidden");
        cfg.cflags.push_back("-Wdeclaration-after-statement");
        cfg.cflags.push_back("-Werror=declaration-after-statement");
        cfg.cflags.push_back("-D_REENTRANT");
        cfg.cflags.push_back("-I/usr/include/dbus-1.0");
        cfg.cflags.push_back("-I/usr/lib/x86_64-linux-gnu/dbus-1.0/include");
        cfg.cflags.push_back("-D_REENTRANT");
        cfg.cflags.push_back("-DHAVE_LINUX_VERSION_H");
        cfg.libs.push_back("dl");
        cfg.libs.push_back("sndio");
        
        config.incToPkg.erase("SDL.h");
        config.common.cflags.push_back("-I.coddle/libs/sdl2/include");
        config.projects.push_back(cfg);
      }
    },
    {
      "sdlpp", [](Config &config, const std::string &version)
      {
        pullGitLib("https://github.com/antonte/sdlpp.git", version);
        config.common.cflags.push_back("-I.coddle/libs/sdlpp");
      }
    }
  };
  auto iter = libs.find(lib);
  if (iter == std::end(libs))
    THROW_ERROR("Library " << lib << " not found");
  return iter->second(config, version);
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
  pullGitLib(&::pullGitLib),
  getLib(&::getLib)
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
  incToPkg["curl/curl.h"].push_back("libcurl");
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
