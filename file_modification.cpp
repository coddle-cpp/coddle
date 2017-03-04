#include "file_modification.hpp"
#include <sys/stat.h>

time_t fileModification(const std::string &fileName)
{
  struct stat buffer;
  if (stat(fileName.c_str(), &buffer) != 0)
    return 0;
  return buffer.st_mtime;
}
