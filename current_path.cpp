#include "current_path.hpp"
#include <unistd.h>

std::string currentPath()
{
  std::string res;
  char *cres = getcwd(nullptr, 0);
  res = cres;
  free(cres);
  return res;
}
