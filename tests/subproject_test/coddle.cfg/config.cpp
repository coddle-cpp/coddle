#include <coddle/config.hpp>

void configure(Config &cfg)
{
  {
    ProjectConfig prj;
    prj.srcDirs.push_back("library");
    prj.targetType = TargetType::StaticLib;
    cfg.projects.push_back(prj);
  }
  {
    ProjectConfig prj;
    prj.srcDirs.push_back("bin");
    cfg.projects.push_back(prj);
  }
  cfg.common.incDirs.push_back(".");
}
