#pragma once
#include "macro.hpp"
#include "ser.hpp"
#include <string>
#include <vector>

#define LIBRARY_TYPES \
  X_MACRO(File)       \
  X_MACRO(Git)        \
  X_MACRO(PkgConfig)  \
  X_MACRO(Lib)        \
  X_MACRO(Framework)  \
  X_MACRO(EmPort)

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
  std::vector<std::string> incdirs;
  std::string libdir;
  std::vector<std::string> dependencies;
  std::string flag; // only for emscripten ports
#define SER_PROPERTY_LIST     \
  SER_PROPERTY(type);         \
  SER_PROPERTY(name);         \
  SER_PROPERTY(path);         \
  SER_PROPERTY(version);      \
  SER_PROPERTY(postClone);    \
  SER_PROPERTY(includes);     \
  SER_PROPERTY(incdir);       \
  SER_PROPERTY(incdirs);      \
  SER_PROPERTY(libdir);       \
  SER_PROPERTY(dependencies); \
  SER_PROPERTY(flag);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

std::string toString(Library::Type);
Library::Type toLibraryType(const std::string &);
