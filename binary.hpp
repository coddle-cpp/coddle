#pragma once
#include "dependency.hpp"
#include <string>

class Binary: public Dependency
{
public:
  Binary(const std::string &directory);
  void resolve();
private:
  std::string directory_;
};
