#pragma once
#include "MurmurHash3.h"
#include "ser.hpp"
#include <condition_variable>
#include <mutex>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

class MemDb
{
public:
  static MemDb &instance();

  std::optional<std::string> lookup(uint32_t hash);
  void insert(uint32_t hash, const std::string &serOut);

private:
  std::unordered_map<uint32_t, std::string> cache;
  std::unordered_set<uint32_t> wip;
  std::mutex mutex;
  std::condition_variable cv;
};
