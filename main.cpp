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
    makeDir(config.artifactsDir);

    Coddle coddle;
    coddle.repository.load(config.remoteRepository, config.remoteVersion);
    coddle.repository.load(config.localRepository);

    return coddle.exec(config);
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
}
