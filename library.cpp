#include "library.hpp"
#include "error.hpp"
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

Library::Type toLibraryType(const std::string &str)
{
  static std::unordered_map<std::string, Library::Type> m = {
#define X_MACRO(x) {#x, Library::Type::x},
    LIBRARY_TYPES
#undef X_MACRO
  };
  auto it = m.find(str);
  if (it == std::end(m))
    THROW_ERROR("Unknown library type: " << str);
  return it->second;
}
