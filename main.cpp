#include "coddle.hpp"
#include "config.hpp"

int main(int arc, char **argv)
{
  Config config(arc, argv);
  config.cflags.push_back("-pthread");
  config.ldflags.push_back("-pthread");
  return coddle(&config);
}
