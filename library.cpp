#include "library.hpp"

Library::Library(const std::string &aName, const std::string &aGit, const std::string &aVersion)
  : name(aName), git(aGit), version(aVersion)
{
}
