#pragma once
#include "MurmurHash3.h"
#include "ser.hpp"
#include <optional>
#include <sstream>
#include <unordered_map>

class MemDb
{
public:
  static MemDb &instance();

  std::unordered_map<uint32_t, std::string> cache;
};
