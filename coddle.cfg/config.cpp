#include <coddle/config.hpp>

void configure(Config &config)
{
  config.multithread = true;;
  config.libs.push_back("dl");
}
