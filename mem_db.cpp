#include "mem_db.hpp"

MemDb &MemDb::instance()
{
  static MemDb db;
  return db;
}
