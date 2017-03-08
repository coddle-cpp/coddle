#pragma once
#include <string>
#include <ctime>
#include <vector>

char getDirSeparator();
std::string getCurrentWorkingDir();
std::string getExecPath();
std::vector<std::string> getFilesList(const std::string &dirPath);
time_t getFileModification(const std::string &);
void changeDir(const std::string &dir);
void exec(const std::string &cmd);
void makeDir(const std::string &);
