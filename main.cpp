#include "coddle.hpp"
#include "config.hpp"

int main(int arc, char **argv)
{
  Config config(arc, argv);
  return coddle(&config);
}
