#pragma once
#include "resolver.hpp"

class Source : public Resolver
{
public:
  using Resolver::Resolver;
  void resolve() override;
};
