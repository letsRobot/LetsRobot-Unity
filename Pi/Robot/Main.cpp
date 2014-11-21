#include "RobotProgram.h"
#include "MessageObserver.h"
#include <signal.h>
#include <iostream>

MessageObserver * sigIntHandlerMessageObserver = 0;

void SigIntHandler(int)
{
   std::cout << std::endl;

   if(sigIntHandlerMessageObserver)
      sigIntHandlerMessageObserver->NewMessage(false, "", "/q");
}

int main()
{
   signal(SIGPIPE, SIG_IGN); // Ignore the signal when a socket is disconnected.
   signal(SIGINT, SigIntHandler);

   RobotProgram program;
   return program.GetResult();
}
