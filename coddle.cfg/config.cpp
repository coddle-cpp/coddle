#include <coddle/config.hpp>

void configure(Config &config)
{
  config.multithread = true;;
  config.common.libs.push_back("dl");
}
