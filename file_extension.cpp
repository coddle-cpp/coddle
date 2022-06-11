#include "file_extension.hpp"

std::string getFileExtension(const std::string &value)
{
  auto p = value.rfind(".");
  if (p == std::string::npos)
    return "";
  return value.substr(p + 1);
}
