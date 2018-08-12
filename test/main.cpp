#include "func.hpp"
#include "test_lib/test_lib.hpp"
#include <curl/curl.h>
#include <iostream>

int main()
{
  std::cout << "Hello world!\n";
  std::cout << "Func: " << func() << "\n";
}
