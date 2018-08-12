#include "coddle.hpp"
#include "config.hpp"
#include "osal.hpp"
#include "repository.hpp"

int main(int argc, char **argv)
{
  Config config(argc, argv);
  config.srcDir = ".";
  config.targetDir = ".";
  config.artifactsDir = ".coddle";
  makeDir(config.artifactsDir);

  Repository repository;
  repository.load(config.remoteRepository, config.remoteVersion);
  repository.load(config.localRepository);

  return coddle(config, repository);
}
