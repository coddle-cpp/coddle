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

template <typename T>
void makePath(std::ostringstream &strm, const T &value)
{
  strm << value;
}

template <typename T, typename... Args>
void makePath(std::ostringstream &strm, const T &value, const Args &...args)
{
  strm << value << getDirSeparator();
  makePath(strm, args...);
}

template <typename... Args>
std::string makePath(const Args &...args)
{
  std::ostringstream strm;
  makePath(strm, args...);
  return strm.str();
}


