#pragma once
#include "mem_db.hpp"
#include <fstream>
#include <iostream>
#include <utility>

template <typename Arg, typename... Args>
void serArgs(OStrm &ost, Arg &&arg, Args &&... args)
{
  ser(ost, arg);
  serArgs(ost, std::forward<Args>(args)...);
}

void serArgs(OStrm &) {}

template <typename R, typename... Args, typename... ArgsU>
R func(R(f)(ArgsU...), Args &&... args)
{
  auto &db = MemDb::instance();
  OStrm ost;
  ser(ost, typeid(f).name());
  serArgs(ost, std::forward<Args>(args)...);
  uint32_t hash;
  MurmurHash3_x86_32(ost.str().data(), ost.str().size(), 0, &hash);
  auto iter = db.cache.find(hash);
  if (iter == std::end(db.cache))
  {
    std::ifstream strm(".coddle/" + std::to_string(hash) + ".artifact");
    if (!strm)
    {
      auto ret = f(args...);
      OStrm ost;
      ser(ost, ret);
      db.cache.emplace(hash, ost.str());
      std::ofstream strm(".coddle/" + std::to_string(hash) + ".artifact");
      strm << ost.str();
      return ret;
    }
    std::ostringstream out;
    out << strm.rdbuf();
    iter = db.cache.emplace(hash, out.str()).first;
  }
  IStrm istrm(iter->second.data(), iter->second.data() + iter->second.size());
  R ret;
  deser(istrm, ret);
  return ret;
}
