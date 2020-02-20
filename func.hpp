#pragma once
#include "file.hpp"
#include "mem_db.hpp"
#include "osal.hpp"
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

class Val
{
public:
  template <typename T>
  constexpr auto operator()(const char *, const T &value) -> void
  {
    if constexpr (internal::IsSerializableClassV<T>)
      value.ser(*this);
    else
      serVal(value);
  }

  auto operator()(const char *, const File &value) -> void
  {
    isValid = isValid && (getFileModification(value.name) == value.modifTime);
  }

  template <typename T>
  constexpr auto serVal(const T &) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>
  {
  }

  auto serVal(const std::string &value) noexcept -> void {}

  template <typename T>
  constexpr auto serVal(const std::vector<T> &value) -> void
  {
    for (auto &&v : value)
    {
      operator()("v", v);
      if (!isValid)
        return;
    }
  }

  template <typename T>
  constexpr auto serVal(const std::unique_ptr<T> &value) -> void
  {
    if (!value)
      return;
    isValid = isValid && operator()("*value", *value);
  }

  template <typename T>
  constexpr auto serVal(const std::optional<T> &value) -> void
  {
    if (!value)
      return;
    operator()("*value", *value);
  }

  bool isValid = true;
};

template <typename T>
constexpr auto validate(const T &value) -> bool
{
  Val v;
  if constexpr (internal::IsSerializableClassV<T>)
    value.ser(v);
  else
    v("value", value);
  return v.isValid;
}

auto validate(const File &value) -> bool
{
  return getFileModification(value.name) == value.modifTime;
}

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

  auto execAndCache = [&]()
  {
    const auto ret = f(args...);
    OStrm ost;
    ser(ost, ret);
    db.cache.emplace(hash, ost.str());
    std::ofstream strm(".coddle/" + std::to_string(hash) + ".artifact", std::ios::binary);
    strm << ost.str();
    return ret;
  };

  if (iter == std::end(db.cache))
  {
    std::ifstream strm(".coddle/" + std::to_string(hash) + ".artifact", std::ios::binary);
    if (!strm)
      return execAndCache();
    std::ostringstream out;
    out << strm.rdbuf();
    iter = db.cache.emplace(hash, out.str()).first;
  }
  IStrm istrm(iter->second.data(), iter->second.data() + iter->second.size());
  R ret;
  deser(istrm, ret);
  if (!validate(ret))
    return execAndCache();
  return ret;
}
