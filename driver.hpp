#pragma once
#include <memory>
#include <string>

class Config;
class Resolver;
class Driver
{
public:
  virtual ~Driver() {}
  virtual std::unique_ptr<Resolver> makeBinaryResolver(Config *) = 0;
  virtual std::unique_ptr<Resolver> makeObjectResolver(const std::string &srcFile, Config *) = 0;
};
