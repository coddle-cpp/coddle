#include "file.hpp"
#include "osal.hpp"
#include <iostream>

File::File(const std::string &name) : name(name), modifTime(getFileModification(name))
{
  std::cout << "File " << name << " modifTime " << modifTime << std::endl;
}