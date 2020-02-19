#pragma once
#include "is_serializable.hpp"
#include "ostrm.hpp"
#include <memory>
#include <typeinfo>

namespace internal
{
  class Schema
  {
  public:
    constexpr Schema(OStrm &strm) : strm(strm) {}

    template <typename T>
    constexpr auto operator()(const char *name, const T &value) -> void
    {
      if constexpr (internal::IsSerializableClassV<T>)
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write("{\n", 2);
        value.ser(*this);
        strm.write("}\n", 2);
      }
      else
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write(typeid(T).name(), strlen(typeid(T).name()));
        strm.write("\n", 1);
      }
    }
    template <typename T>
    constexpr auto operator()(const char *name, const std::vector<T> &) -> void
    {
      if constexpr (internal::IsSerializableClassV<T>)
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write("{\n", 2);
        T{}.ser(*this);
        strm.write("}", 1);
      }
      else
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write(typeid(T).name(), strlen(typeid(T).name()));
      }
      strm.write("[]\n", 3);
    }
    constexpr auto operator()(const char *name, const uint16_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":uint16_t\n", strlen(":uint16_t\n"));
    }
    constexpr auto operator()(const char *name, const uint32_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":uint32_t\n", strlen(":uint32_t\n"));
    }
    constexpr auto operator()(const char *name, const uint64_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":uint64_t\n", strlen(":uint64_t\n"));
    }
    constexpr auto operator()(const char *name, const int16_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":int16_t\n", strlen(":int16_t\n"));
    }
    constexpr auto operator()(const char *name, const int32_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":int32_t\n", strlen(":int32_t\n"));
    }
    constexpr auto operator()(const char *name, const int64_t &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":int64_t\n", strlen(":int64_t\n"));
    }
    constexpr auto operator()(const char *name, const std::string &) -> void
    {
      strm.write(name, strlen(name));
      strm.write(":std::string\n", strlen(":std::string\n"));
    }
    template <typename T>
    constexpr auto operator()(const char *name, const std::unique_ptr<T> &) -> void
    {
      if constexpr (internal::IsSerializableClassV<T>)
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write("{\n", 2);
        T{}.ser(*this);
        strm.write("}", 1);
      }
      else
      {
        strm.write(name, strlen(name));
        strm.write(":", 1);
        strm.write(typeid(T).name(), strlen(typeid(T).name()));
      }
      strm.write("*\n", 2);
    }

  private:
    OStrm &strm;
  };
} // namespace internal

template <typename T>
constexpr auto schema(OStrm &strm, const T &value) -> void
{
  internal::Schema s(strm);
  if constexpr (internal::IsSerializableClassV<T>)
  {
    strm.write("value:{\n", strlen("value:{\n"));
    value.ser(s);
    strm.write("}\n", 2);
  }
  else
    s("value", value);
}
