#include "file_name.hpp"

std::string fileName(const std::string &fileName)
{
  auto idx = fileName.rfind('/');
  return idx != std::string::npos ? fileName.substr(idx + 1) : "";
}
