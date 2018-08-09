#include "config.hpp"
#include "coddle.hpp"
#include "repository.hpp"

int main(int argc, char **argv)
{
  Config config(argc, argv);
  Repository repository;
  repository.load("remote", config.remoteRepository, config.remoteVersion);
  repository.load("local", config.localRepository, config.localVersion);

  return coddle(&config);
}
