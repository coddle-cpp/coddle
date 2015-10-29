#include "coddle.hpp"
#include "binary.hpp"
#include "dependency.hpp"
#include <iostream>
#include <stdexcept>


int coddle(const std::string &directory)
{
  try
  {
    Dependency::get(directory)->resolve();
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
