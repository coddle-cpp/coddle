#include "binary.hpp"
#include "error.hpp"
#include "exec.hpp"
#include "object.hpp"
#include <iostream>
#include <sstream>

void Binary::resolve()
{
  std::string objList;
  bool hasMain = false;
  for (auto &dependency: dependencyList)
  {
    if (auto object = dynamic_cast<Object *>(dependency.get()))
      if (object->hasMain())
        hasMain = true;
    objList += dependency->fileName + " ";
  }
  try
  {
    std::ostringstream strm;
    if (hasMain)
      strm << "g++ -pthread " << objList << "-o " << fileName;
    else
      strm << "ar rv lib" << fileName << ".a " << objList;
    std::cout << strm.str() << std::endl;
    exec(strm.str());
  }
  catch (std::exception &e)
  {
    ERROR("coddle: *** [" << fileName << "] " << e.what())
  }
}

void Binary::wait()
{
}
