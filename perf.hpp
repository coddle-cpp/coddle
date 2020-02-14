#pragma once
#include <chrono>
#include <string>

class Perf
{
public:
  Perf(const std::string &);
  ~Perf();
private:
  std::string msg;
  static std::string level;
  decltype(std::chrono::high_resolution_clock::now()) t0;
};
