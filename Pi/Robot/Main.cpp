#include "RobotProgram.h"
#include <signal.h>

int main()
{
   signal(SIGPIPE, SIG_IGN); // Ignore the signal when a socket is disconnected.

   RobotProgram program;
   return program.GetResult();
}
