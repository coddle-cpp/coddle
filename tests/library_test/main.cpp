#include <json/json.h>
#include <iostream>
#include <sstream>

int main()
{
  std::istringstream strm("{ \"aaa\": 10 }");
  Json::Value v;
  strm >> v;
  std::cout << v["aaa"] << std::endl;
}
