#include <coddle/config.hpp>

void configure(Config &cfg)
{
  cfg.addProject(cfg, "library", TargetType::StaticLib);
  cfg.addProject(cfg, "bin", TargetType::Executable);
}
