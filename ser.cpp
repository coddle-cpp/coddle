#include "ser.hpp"

auto Ser::serVal(const std::string &value) noexcept -> void
{
  auto sz{static_cast<int32_t>(value.size())};
  strm.write((char *)&sz, sizeof(sz));
  strm.write((char *)value.data(), sz);
}

auto Deser::deserVal(std::string &value) noexcept -> void
{
  int32_t sz{};
  strm.read((char *)&sz, sizeof(sz));
  value.resize(sz);
  strm.read((char *)value.data(), sz);
}
