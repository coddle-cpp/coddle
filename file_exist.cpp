#include "file_exist.hpp"
#ifdef _WIN32
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif

bool isFileExist(const std::string &fileName)
{
  return access(fileName.c_str(), 0) == 0;
}
