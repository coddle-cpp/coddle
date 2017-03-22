#include <coddle/coddle.hpp>
#include <coddle/config.hpp>

int main(int argc, char **argv)
{
  Config config(argc, argv);
  config.cflags.push_back("-pthread");
  config.ldflags.push_back("-pthread");
  return coddle(&config);
}
