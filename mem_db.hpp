#pragma once
#include "MurmurHash3.h"
#include "ser.hpp"
#include <mutex>
#include <optional>
#include <sstream>
#include <unordered_map>

class MemDb
{
public:
  static MemDb &instance();

  std::optional<std::string> lookup(uint32_t hash) const;
  void insert(uint32_t hash, const std::string &serOut);

private:
  mutable std::unordered_map<uint32_t, std::string> cache;
  mutable std::mutex mutex;
};
