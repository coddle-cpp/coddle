#pragma once
#include <string>

class OStrm
{
public:
  OStrm() = default;
  OStrm(const OStrm &) = delete;
  OStrm &operator=(const OStrm &) = delete;
  constexpr auto write(const char *b, size_t sz) noexcept -> void
  {
    for (; sz > 0; --sz, ++b)
      buff.push_back(*b);
  }
  std::string &str() { return buff; }

private:
  std::string buff;
};
