#include "config.hpp"
#include "file_exist.hpp"
#include "file_name.hpp"
#include "gcc_binary.hpp"
#include "gcc_object.hpp"
#include "osal.hpp"
#include "source.hpp"
#include "vs_binary.hpp"
#include "vs_object.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

Config::Config(int argc, char **argv):
  binaryFactory{[](Config *config)
  {
    auto target = config->target;
    if (target.empty())
    {
      target = fileName(getCurrentWorkingDir());
#ifdef _WIN32
      target += ".exe";
#endif
    }
#ifdef _WIN32
    return std::make_unique<Vs::Binary>(target, config);
#else
    return std::make_unique<Gcc::Binary>(target, config);
#endif
  }},
  objectFactory{[](const std::string &srcFileName, Config *config)
    {
#ifdef _WIN32
      return std::make_unique<Vs::Object>(srcFileName, config);
#else
      return std::make_unique<Gcc::Object>(srcFileName, config);
#endif
    }
  },
  addDependency{[this](Dependency *obj, const std::string &srcFileName)
    {
#ifdef _WIN32
      obj->add(std::make_unique<Source>(srcFileName, config));
      if (isFileExist(".coddle" + getDirSeparator() + srcFileName + ".obj.inc"))
      {
        std::ifstream f(".coddle" + getDirSeparator() + srcFileName + ".obj.inc");
        std::string header;
        while (std::getline(f, header))
        {
          auto p = header.find(":");
          if (p == std::string::npos)
            continue;
          p = header.find(":", p + 1);
          if (p == std::string::npos)
            continue;
          ++p;
          while (p < header.size() && header[p] == ' ')
            ++p;
          header = header.substr(p);
          if (!header.empty())
            obj->add(std::make_unique<Source>(header, config));
        }
      }
#else
      if (!isFileExist(".coddle" + getDirSeparator() + srcFileName + ".o.mk"))
        obj->add(std::make_unique<Source>(srcFileName, this));
      else
      {
        std::string str = [](const std::string &file)
          {
            std::ifstream f(".coddle" + getDirSeparator() + file + ".o.mk");
            std::ostringstream strm;
            f >> strm.rdbuf();
            return strm.str();
          }(srcFileName);
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
            obj->add(std::make_unique<Source>(srcFile, this));
#endif
      }
    }}
{
  for (auto i = 0; i < argc; ++i)
    args.push_back(argv[0]);
#ifndef _WIN32
  cflags.push_back("-Wall");
  cflags.push_back("-Wextra");
  cflags.push_back("-march=native");
  cflags.push_back("-gdwarf-3");
  cflags.push_back("-std=c++1y");
  cflags.push_back("-O3");
  cflags.push_back("-g");
#else
  cflags.push_back("/EHsc");
  cflags.push_back("/W4");
  cflags.push_back("/Ox");
  cflags.push_back("/nologo");
  ldflags.push_back("/nologo");
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

bool Config::configured() const
{
  return args[0].find("coddle.cfg") != std::string::npos;
}

std::string Config::execPath() const
{
  return getExecPath();
}

void Config::configureForConfig()
{
#ifndef _WIN32
  target = "coddle";
  cflags.push_back("-I ~/.coddle/include");
  ldflags.push_back("-L ~/.coddle/lib");
  ldflags.push_back("-lcoddle");
  if (std::find(std::begin(cflags), std::end(cflags), "-pthread") == std::end(cflags))
    cflags.push_back("-pthread");
  if (std::find(std::begin(ldflags), std::end(ldflags), "-pthread") == std::end(ldflags))
    ldflags.push_back("-pthread");
#else
  // TODO
#endif
}
