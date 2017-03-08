#include "source.hpp"
#include "error.hpp"
#include "file_exist.hpp"
#include <stdexcept>

void Source::resolve()
{
  if (!isFileExist(fileName))
    THROW_ERROR("coddle: *** No rule to make target '" << fileName << "'. Stop.");
}
