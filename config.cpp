#include "config.hpp"
#include <iostream>
#include <unistd.h>

Config::Config(int argc, char **argv)
{
  for (auto i = 0; i < argc; ++i)
    args.push_back(argv[0]);
  cflags.push_back("-Wall");
  cflags.push_back("-Wextra");
  cflags.push_back("-march=native");
  cflags.push_back("-gdwarf-3");
  cflags.push_back("-std=c++1y");
  cflags.push_back("-O3");
  cflags.push_back("-g");

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

std::string Config::exe() const
{
  char buf[512];
  int count = readlink("/proc/self/exe", buf, sizeof(buf));
  if (count >= 0) {
    buf[count] = '\0';
    return buf;
  }
  return std::string();
}
