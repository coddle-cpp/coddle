#include "source.hpp"
#include "error.hpp"
#include "file_exist.hpp"
#include <stdexcept>

void Source::resolve()
{
  if (!isFileExist(fileName))
    ERROR("coddle: *** No rule to make target '" << fileName << "'. Stop.");
}
