#pragma once
#include "macro.hpp"
#include "ser.hpp"
#include <ctime>
#include <string>

class File
{
public:
  explicit File(const std::string &name = {});
  std::string name;
  time_t modifTime;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(name);     \
  SER_PROPERTY(modifTime);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};
