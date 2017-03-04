#include "file_exist.hpp"
#include <fstream>

bool isFileExist(const std::string &fileName)
{
  return std::ifstream(fileName).good();
}
