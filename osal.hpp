#pragma once
#include <string>
#include <ctime>

std::string currentPath();
std::string getExecPath();
time_t fileModification(const std::string &);
void changeDir(const std::string &dir);
void exec(const std::string &cmd);
void makeDir(const std::string &);
