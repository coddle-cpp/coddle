#pragma once

#include "resolver.hpp"

class Solution: public Resolver
{
public:
  using Resolver::Resolver;
  void resolve() override;
};
