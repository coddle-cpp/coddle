#pragma once
#include <sstream>
#include <stdexcept>

#define THROW_ERROR(x) {                        \
    std::ostringstream strm;                    \
    strm << x;                                  \
    throw std::runtime_error(strm.str());       \
  }
