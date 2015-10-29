#pragma once
#include <string>

class Coddle
{
public:
  Coddle(const std::string &path);
  int operator()();
private:
  std::string path_;
};
