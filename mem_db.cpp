#include "mem_db.hpp"

MemDb &MemDb::instance()
{
  static MemDb db;
  return db;
}

std::optional<std::string> MemDb::lookup(uint32_t hash) const
{
  const std::lock_guard<std::mutex> lock(mutex);
  const auto iter = cache.find(hash);
  if (iter == std::end(cache))
    return std::nullopt;
  return iter->second;
}

void MemDb::insert(uint32_t hash, const std::string &serOut)
{
  const std::lock_guard<std::mutex> lock(mutex);
  cache.emplace(hash, serOut);
}
