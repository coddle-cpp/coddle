#include "path_relative.hpp"

bool isPathRelative(const std::string &path)
{
  if (path.empty())
    return true;
  if (path[0] == '/')
    return false;
  if (path[0] == '\\')
    return false;
#ifdef _WIN32
  if (path.find(':') == 1)
    return false;
#endif
  return true;
}
