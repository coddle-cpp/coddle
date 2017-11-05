#pragma once
#include <memory>
#include <string>

class Config;
struct ProjectConfig;
class Resolver;
class Driver
{
public:
  virtual ~Driver() {}
  virtual std::unique_ptr<Resolver> makeBinaryResolver(Config *, ProjectConfig *) = 0;
  virtual std::unique_ptr<Resolver> makeObjectResolver(const std::string &srcFile,
                                                       Config *,
                                                       ProjectConfig *) = 0;
};
