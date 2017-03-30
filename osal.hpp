#pragma once
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

std::string getDirSeparator();
std::string getCurrentWorkingDir();
std::string getExecPath();
std::vector<std::string> getFilesList(const std::string &dirPath);
time_t getFileModification(const std::string &);
void changeDir(const std::string &dir);
void exec(const std::string &cmd);
void makeDir(const std::string &);

class SharedLib
{
public:
  SharedLib(const std::string &file);
  void *symbol(const std::string &sym);
  ~SharedLib();
private:
  void *handle;
};
