#pragma once
#include "resolver.hpp"
#include <vector>

namespace Gcc
{
class Object;
class Binary: public Resolver
{
public:
  using Resolver::Resolver;
  void preResolve() override;
  void resolve() override;
};
}
