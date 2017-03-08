#include "file_name.hpp"

std::string fileName(const std::string &path)
{
  auto idx = path.find_last_of("/\\");
  return idx != std::string::npos ? path.substr(idx + 1) : "";
}
