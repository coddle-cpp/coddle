#include "coddle.hpp"
#include "config.hpp"
#include "file_exist.hpp"
#include "make_path.hpp"
#include "osal.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  if (isFileExist("coddle.cfg"))
  {
    Config config(argc, argv);
    std::cout << "coddle: Entering directory `coddle.cfg'" << std::endl;
    changeDir("coddle.cfg");
    config.multithread = true;
    ProjectConfig project;
    project.srcDirs.push_back(".");
    project.targetType = TargetType::SharedLib;
    config.projects.push_back(project);
    auto res = coddle(&config);
    std::cout << "coddle: Leaving directory `coddle.cfg'" << std::endl;
    changeDir("..");
    if (res != 0)
      return res;
  }
  Config config(argc, argv);
  if (isFileExist(makePath("coddle.cfg", "libcoddle.cfg.so")))
  {
    SharedLib lib(makePath("coddle.cfg", "libcoddle.cfg.so"));
    auto configure = (void (*)(Config &))lib.symbol("_Z9configureR6Config");
    if (!configure)
    {
      std::cerr << "coddle: undefined reference to `configure(Config&)'\n";
      return 2;
    }
    configure(config);
  }
  auto hasUserProject = false;
  auto hasLibraryProject = false;
  for (const auto &p : config.projects)
  {
    if (p.target.find(".coddle/") != 0)
      hasUserProject = true;
    if (p.targetType == TargetType::StaticLib || p.targetType == TargetType::SharedLib)
      hasLibraryProject = true;
    if (hasUserProject && hasLibraryProject)
      break;
  }
  if (!hasUserProject)
  {
    ProjectConfig project;
    project.srcDirs.push_back(".");
    config.projects.push_back(project);
  }
  if (hasLibraryProject)
  {
    config.common.libDirs.push_back(makePath(".coddle", "libs", "usr", "local", "lib"));
    config.common.incDirs.push_back(".");
  }
  return coddle(&config);
}
