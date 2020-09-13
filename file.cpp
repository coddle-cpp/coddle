#include "file.hpp"
#include "osal.hpp"
#include <iostream>

File::File(const std::string &name) : name(name), modifTime(getFileModification(name))
{
  if (!name.empty())
    std::cout << "File " << name << " modifTime " << modifTime << std::endl;
}