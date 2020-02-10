#pragma once
#include <string>
#include <vector>

#define LIBRARY_TYPES \
  X_MACRO(File)       \
  X_MACRO(Git)        \
  X_MACRO(PkgConfig)  \
  X_MACRO(Lib)        \
  X_MACRO(Framework)

class Library
{
public:
  enum class Type {
#define X_MACRO(x) x,
    LIBRARY_TYPES
#undef X_MACRO
  };
  Type type;
  std::string name;
  std::string path;
  std::string version; // only for git
  std::string postClone;
  std::vector<std::string> includes;
  std::string incdir;
  std::string libdir;
  std::vector<std::string> dependencies;
};

std::string toString(Library::Type);
Library::Type toLibraryType(const std::string&);
