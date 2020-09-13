#include "mem_db.hpp"
#include <fstream>

MemDb &MemDb::instance()
{
  static MemDb db;
  return db;
}

std::optional<std::string> MemDb::lookup(uint32_t hash)
{
  std::unique_lock<std::mutex> lock(mutex);
  for (;;)
  {
    const auto iter = cache.find(hash);
    if (iter != std::end(cache))
      return iter->second;
    std::ifstream strm(".coddle/" + std::to_string(hash) + ".artifact", std::ios::binary);
    if (!strm)
    {
      if (wip.find(hash) != std::end(wip))
      {
        cv.wait(lock);
        continue;
      }
      wip.insert(hash);
      return std::nullopt;
    }
    std::ostringstream out;
    out << strm.rdbuf();
    cache.emplace(hash, out.str());
    return out.str();
  }
}

void MemDb::insert(uint32_t hash, const std::string &serOut)
{
  const std::lock_guard<std::mutex> lock(mutex);
  cache.emplace(hash, serOut);
  {
    std::ofstream strm(".coddle/" + std::to_string(hash) + ".artifact", std::ios::binary);
    strm << serOut;
  }
  wip.erase(hash);
  cv.notify_all();
}
