#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>

class IStrm
{
public:
  constexpr IStrm(const char *b, const char *e) noexcept : b(b), e(e) {}
  IStrm(const IStrm &) = delete;
  IStrm &operator=(const IStrm &) = delete;
  constexpr auto read(char *buff, const std::size_t sz) noexcept -> size_t
  {
    auto cnt = sz;
    for (; b != e && cnt > 0; ++b, --cnt, ++buff)
      *buff = *b;
    return sz - cnt;
  }

private:
  const char *b;
  const char *e;
};
