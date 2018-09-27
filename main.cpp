#include "coddle.hpp"
#include "config.hpp"
#include "osal.hpp"
#include "repository.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  try
  {
    Config config(argc, argv);
    config.srcDir = ".";
    config.targetDir = ".";
    config.artifactsDir = ".coddle";

    Coddle coddle;
    coddle.repository.load(config.remoteRepository, config.remoteVersion);
    coddle.repository.load(config.localRepository);

    coddle.build(config);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch (int e)
  {
    return e;
  }
}
