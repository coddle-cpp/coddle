#include "osal.hpp"
#include "error.hpp"
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <limits.h>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

std::string currentPath()
{
  std::string res;
  char *cres = getcwd(nullptr, 0);
  res = cres;
  free(cres);
  return res;
}

std::string getExecPath()
{
  char buf[PATH_MAX];
#ifdef __APPLE__
  uint32_t size = sizeof(buf);
  if (_NSGetExecutablePath(buf, &size) == 0)
    return buf;
#else
  int count = readlink("/proc/self/exe", buf, sizeof(buf));
  if (count >= 0)
    return buf;
#endif
  return std::string();
}

time_t fileModification(const std::string &fileName)
{
  struct stat buffer;
  if (stat(fileName.c_str(), &buffer) != 0)
    return 0;
  return buffer.st_mtime;
}

void changeDir(const std::string &dir)
{
  ::chdir(dir.c_str());
}

void exec(const std::string &cmd)
{
  auto res = system(cmd.c_str());
  if (res != 0)
  {
    if (WIFSIGNALED(res) && (WTERMSIG(res) == SIGINT || WTERMSIG(res) == SIGQUIT))
      ERROR("Interrupt");
    if (WIFEXITED(res))
      ERROR("Error " << WEXITSTATUS(res));
  }
}

void makeDir(const std::string &dir)
{
  std::istringstream strm(dir);
  std::string subDir;
  std::string tmp;
  while (std::getline(strm, tmp, '/'))
  {
    subDir += tmp;
    subDir += "/";
    auto res = mkdir(subDir.c_str(), 0777);
    if (res != 0)
    {
      auto err = errno;
      if (err != EEXIST)
        ERROR("makeDir(" << dir << "): " << strerror(err));
    }
  }
}

std::vector<std::string> getFilesList(const std::string &dirPath)
{
  std::vector<std::string> res;
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(dirPath.c_str())) != NULL)
  {
    while ((ent = readdir (dir)) != NULL)
      if (ent->d_type == DT_LNK || ent->d_type == DT_REG)
        res.push_back(ent->d_name);
    closedir(dir);
  }
  return res;
}
