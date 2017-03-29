#pragma once
#include "driver.hpp"
#include <memory>
#include <string>

class GccDriver: public Driver
{
public:
  std::unique_ptr<Resolver> makeBinaryResolver(Config *) override;
  std::unique_ptr<Resolver> makeObjectResolver(const std::string &srcFile, Config *) override;
};
