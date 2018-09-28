#include "library.hpp"
#include "error.hpp"
#include <algorithm>
#include <unordered_map>

std::string toString(Library::Type type)
{
  switch (type)
  {
#define X_MACRO(x) \
  case Library::Type::x: return #x;
    LIBRARY_TYPES
#undef X_MACRO
  }
}

static std::string toLower(std::string str)
{
  std::transform(std::begin(str), std::end(str), std::begin(str), ::tolower);
  return str;
}

Library::Type toLibraryType(const std::string &str)
{
  static std::unordered_map<std::string, Library::Type> m = {
#define X_MACRO(x) {toLower(#x), Library::Type::x},
    LIBRARY_TYPES
#undef X_MACRO
  };
  auto it = m.find(str);
  if (it == std::end(m))
    THROW_ERROR("Unknown library type: " << str);
  return it->second;
}
