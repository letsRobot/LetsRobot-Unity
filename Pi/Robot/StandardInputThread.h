#ifndef StandardInputThread_h
#define StandardInputThread_h

#include "MessageObserver.h"
#include "Stoppable.h"
#include <Thread.h>
#include <string>
#include <iostream>
#include <cassert>

class StandardInputThread
   : public Thread
{
   public:
      StandardInputThread(MessageObserver * messageObserver, Stoppable * stoppableProgram)
         : messageObserver(messageObserver),
           stoppableProgram(stoppableProgram)
      {
         assert(messageObserver);
         assert(stoppableProgram);

         Start();
      }

      ~StandardInputThread() noexcept
      {
         Stop();
         Join();
      }

   private:
      void Run()
      {
         try
         {
            std::cin.sync_with_stdio(false); // This turns on buffering of the standard input in GCC. In GCC this causes in_avail() to return the number of characters in the buffer.

            while(!stopped)
            {
               const auto line = ReadLine();

               if(!stopped)
                  messageObserver->NewMessage(false, "", line.c_str());
            }
         }
         catch(...)
         {
            stoppableProgram->Stop();
            throw;
         }
      }

      std::string ReadLine()
      {
         std::string line;

         while(!stopped)
         {
            if(!std::cin.rdbuf()->in_avail()) // Wait for input to become available. Using in_avail() in this way is not portable and may not even work in GCC in the future.
            {
               Thread::Sleep(100);
               continue;
            }

            const char c = std::cin.get();

            if(c == '\n') // Return after a new line character.
               return line;
            else if(c != '\r') // Add any character to the linethat is not a new line or a carriage return.
               line += c;
         }

         return "";
      }

      MessageObserver * const messageObserver;
      Stoppable * const stoppableProgram;
};

#endif
