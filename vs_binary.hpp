#pragma once
#include "resolver.hpp"
#include <vector>

namespace Vs
{
class Object;
class Binary: public Resolver
{
public:
  using Resolver::Resolver;
  void resolve() override;
};
}
