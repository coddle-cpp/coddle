#include "coddle.hpp"
#include <iostream>

int main()
{
  try
  {
    coddle();
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
