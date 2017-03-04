#include "exec.hpp"
#include "error.hpp"
#include <stdexcept>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void exec(const std::string &cmd)
{
  auto res = system(cmd.c_str());
  if (res != 0)
  {
    if (WIFSIGNALED(res) && (WTERMSIG(res) == SIGINT || WTERMSIG(res) == SIGQUIT))
      ERROR("Interrupt");
    if (WIFEXITED(res))
      ERROR("Error " << WEXITSTATUS(res));
  }
}
