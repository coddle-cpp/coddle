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
bool isDirExist(const std::string &dir);
void exec(const std::string &cmd);
void execShowCmd(const std::string &cmd);

template <typename T>
void buildCmd(std::ostringstream &strm, const T &value)
{
  strm << value;
}

template <typename T, typename... Args>
void buildCmd(std::ostringstream &strm, const T &value, const Args &... args)
{
  strm << value << " ";
  buildCmd(strm, args...);
}

template <typename... Args>
void exec(const Args &... args)
{
  std::ostringstream strm;
  buildCmd(strm, args...);
  exec(strm.str());
}

template <typename... Args>
void execShowCmd(const Args &... args)
{
  std::ostringstream strm;
  buildCmd(strm, args...);
  execShowCmd(strm.str());
}

void makeDir(const std::string &);

