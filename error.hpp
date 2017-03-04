#pragma once
#include <sstream>
#include <stdexcept>

#define ERROR(x) {                              \
    std::ostringstream strm;                    \
    strm << x;                                  \
    throw std::runtime_error(strm.str());       \
  }
