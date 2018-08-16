#include "library.hpp"

Library::Library(Type aType,
                 const std::string &aName,
                 const std::string &aPath,
                 const std::string &aVersion,
                 const std::string &aPostClone)
  : type(aType), name(aName), path(aPath), version(aVersion), postClone(aPostClone)
{
}
