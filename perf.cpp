#include "perf.hpp"
#include <iostream>

#if 0

Perf::Perf(const std::string &msg) : msg(msg), t0(std::chrono::high_resolution_clock::now())
{
  std::clog << level << msg << std::endl;
  level += ">";
}

Perf::~Perf()
{
  auto t1 = std::chrono::high_resolution_clock::now();
  level.resize(level.size() - 1);
  std::clog << level << msg << ": "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
}

#else
Perf::Perf(const std::string &) {}
Perf::~Perf() {}
#endif

std::string Perf::level;
