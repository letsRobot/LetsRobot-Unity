#include "StreamerThread.h"
#include <signal.h>
#include <iostream>
#include <string>
#include <stdexcept>

StreamerThread * streamerPointer = 0;

void SignalHandler(int signalNumber)
{
   if(signalNumber == SIGINT && streamerPointer)
   {
      std::cout << std::endl;
      streamerPointer->Stop();
      streamerPointer = 0;
   }
}

int main(int argc, char * * argv)
{
   if(argc != 3)
   {
      std::cerr << "Usage: Stream filename port" << std::endl;
      std::cerr << "Use ctrl+c to exit the program." << std::endl;
      return 1;
   }

   try
   {
      signal(SIGPIPE, SIG_IGN); // Make sure a disconnected socket won't give us a signal

      try
      {
         const auto port = std::stoi(argv[2]);
         if(port < 0)
            throw std::invalid_argument("");
      }
      catch(std::invalid_argument &)
      {
         throw "port must be a number.";
      }

      StreamerThread streamer(argv[1], atoi(argv[2]));

      streamerPointer = &streamer;
      signal(SIGINT, SignalHandler);

      streamer.Join();
      streamer.ThrowException();
   }
   catch(Exception & e)
   {
      std::cerr << "Error." << std::endl;
      std::cerr << e.Message() << std::endl;
      std::cerr << e.ExtendedMessage() << std::endl;
      return 1;
   }
   catch(std::bad_alloc &)
   {
      std::cerr << "Error." << std::endl << "Out of memory." << std::endl;
      return 1;
   }
   catch(std::exception & e)
   {
      std::cerr << "Error." << std::endl;
      std::cerr << e.what() << std::endl;
      return 1;
   }
   catch(const char * str)
   {
      std::cerr << "Error." << std::endl;
      std::cerr << str << std::endl;
      return 1;
   }
   catch(...)
   {
      std::cerr << "Unknown error." << std::endl;
      return 1;
   }
}

