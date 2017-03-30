#pragma once
#include "osal.hpp"

template <typename T>
void makePath(std::ostringstream &strm, const T &value)
{
  strm << value;
}

template <typename T, typename... Args>
void makePath(std::ostringstream &strm, const T &value, const Args &...args)
{
  if ([&value]()
      {
        std::ostringstream strm;
        strm << value;
        return strm.str();
      }() != ".")
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
