#pragma once
#include "dependency.hpp"
#include <vector>

namespace Gcc
{
class Object;
class Binary: public Dependency
{
public:
  using Dependency::Dependency;
  void resolve() override;
};
}
