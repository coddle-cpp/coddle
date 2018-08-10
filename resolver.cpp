#include "resolver.hpp"

void Resolver::dependsOf(const std::string& fileName)
{
  dependencies.insert(fileName);
}
