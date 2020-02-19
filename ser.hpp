#pragma once
#include "is_serializable.hpp"
#include "istrm.hpp"
#include "ostrm.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Ser;
class Deser;

class Ser
{
public:
  constexpr Ser(OStrm &strm) : strm(strm) {}

  template <typename T>
  constexpr auto operator()(const char *, const T &value) -> void
  {
    if constexpr (internal::IsSerializableClassV<T>)
      value.ser(*this);
    else
      serVal(value);
  }

  template <typename T>
  constexpr auto serVal(const T &value) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>
  {
    strm.write((char *)&value, sizeof(value));
  }

  auto serVal(const std::string &value) noexcept -> void;

  template <typename T>
  constexpr auto serVal(const std::vector<T> &value) -> void
  {
    int32_t sz = value.size();
    operator()("sz", sz);
    for (auto &&v : value)
      operator()("v", v);
  }

  template <typename T>
  constexpr auto serVal(const std::unique_ptr<T> &value) -> void
  {
    int32_t isNull = value ? 0 : 1;
    operator()("isNull", isNull);
    if (value)
      operator()("*value", *value);
  }

  template <typename T>
  constexpr auto serVal(const std::optional<T> &value) -> void
  {
    int32_t isNull = value ? 0 : 1;
    operator()("isNull", isNull);
    if (value)
      operator()("*value", *value);
  }

private:
  OStrm &strm;
};

class Deser
{
public:
  constexpr Deser(IStrm &strm) : strm(strm) {}

  template <typename T>
  constexpr auto deserVal(T &value) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>
  {
    strm.read((char *)&value, sizeof(value));
  }

  auto deserVal(std::string &value) noexcept -> void;

  template <typename T>
  constexpr auto operator()(const char *, T &value) -> void
  {
    if constexpr (internal::IsSerializableClassV<T>)
      value.deser(*this);
    else
      deserVal(value);
  }

  template <typename T>
  constexpr auto deserVal(std::vector<T> &value) -> void
  {
    value.clear();
    int32_t sz{};
    operator()("sz", sz);
    value.reserve(sz);
    for (auto i = 0; i < sz; ++i)
    {
      T &v = value.emplace_back();
      operator()("v", v);
    }
  }

  template <typename T>
  constexpr auto deserVal(std::unique_ptr<T> &value) -> void
  {
    int32_t isNull{};
    operator()("isNull", isNull);
    if (isNull == 1)
    {
      value = nullptr;
      return;
    }

    value = std::make_unique<T>();
    operator()("*value", *value);
  }

  template <typename T>
  constexpr auto deserVal(std::optional<T> &value) -> void
  {
    int32_t isNull{};
    operator()("isNull", isNull);
    if (isNull == 1)
    {
      value = std::nullopt;
      return;
    }

    value = T{};
    operator()("*value", *value);
  }

private:
  IStrm &strm;
};

template <typename T>
constexpr auto ser(OStrm &strm, const T &value) -> void
{
  Ser s(strm);
  if constexpr (internal::IsSerializableClassV<T>)
    value.ser(s);
  else
    s("value", value);
}

template <typename T>
constexpr auto deser(IStrm &strm, T &value) -> void
{
  Deser s(strm);
  if constexpr (internal::IsSerializableClassV<T>)
    value.deser(s);
  else
    s("value", value);
}
