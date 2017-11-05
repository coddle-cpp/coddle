#pragma once
#include "driver.hpp"
#include <memory>
#include <string>

class VsDriver : public Driver
{
public:
  std::unique_ptr<Resolver> makeBinaryResolver(Config *, ProjectConfig *) override;
  std::unique_ptr<Resolver> makeObjectResolver(const std::string &srcFile,
                                               Config *,
                                               ProjectConfig *) override;
};
