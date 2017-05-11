#include "coddle/config.hpp"

void configure(Config &cfg)
{
  cfg.getLib(cfg, "jsoncpp", "1.8.0");
}
