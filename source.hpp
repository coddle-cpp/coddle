#pragma once
#include "dependency.hpp"

class Source: public Dependency
{
public:
  using Dependency::Dependency;
  void resolve() override;
};
