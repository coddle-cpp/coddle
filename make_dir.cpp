#include "make_dir.hpp"
#include "error.hpp"
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


void makeDir(const std::string &dir)
{
  std::istringstream strm(dir);
  std::string subDir;
  std::string tmp;
  while (std::getline(strm, tmp, '/'))
  {
    subDir += tmp;
    subDir += "/";
    auto res = mkdir(subDir.c_str(), 0777);
    if (res != 0)
    {
      auto err = errno;
      if (err != EEXIST)
        ERROR("makeDir(" << dir << "): " << strerror(err));
    }
  }
}
