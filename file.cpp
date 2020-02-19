#include "file.hpp"
#include "osal.hpp"

File::File(const std::string &name) : name(name), modifTime(getFileModification(name)) {}