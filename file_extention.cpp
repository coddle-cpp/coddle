#include "file_extention.hpp"

std::string getFileExtention(const std::string &value)
{
  auto p = value.rfind(".");
  if (p == std::string::npos)
    return "";
  return value.substr(p + 1);
}
