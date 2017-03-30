#include "coddle.hpp"
#include "config.hpp"
#include "file_exist.hpp"
#include "osal.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  if (isFileExist("coddle.cfg"))
  {
    Config config(argc, argv);
    std::cout << "coddle: Entering directory `coddle.cfg'" << std::endl;
    changeDir("coddle.cfg");
    config.multithread = true;
    config.targetType = TargetType::SharedLib;
    config.cflags.push_back("-I ~/.coddle/include");
    coddle(&config);
    std::cout << "coddle: Leaving directory `coddle.cfg'" << std::endl;
    changeDir("..");
  }
  Config config(argc, argv);
  if (isFileExist("coddle.cfg/libcoddle.cfg.so"))
  {
    SharedLib lib("coddle.cfg/libcoddle.cfg.so");
    auto configure = (void (*)(Config &))lib.symbol("_Z9configureR6Config");
    configure(config);
  }
  return coddle(&config);
}
