#ifndef UnityThreads_h
#define UnityThreads_h

#include "Unity.h"
#include "Stoppable.h"
#include "ThreadSafeQueue.h"
#include "PackageAssembler.h"
#include "MessageObserver.h"
#include <TcpSocket.h>
#include <cstdio>
#include <string>
#include <thread>
#include <mutex>
#include <ctime>
#include <exception>
#include <memory>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>

class UnityThreads
   : public Unity
{
   public:
      UnityThreads(uint16_t port, Stoppable * stoppableProgram, MessageObserver * messageObserver)
      : stopped(false),
        inputHasConnected(false),
        outputHasConnected(false),
        badAlloc(false),
        serverSocket(port),
        stoppableProgram(stoppableProgram),
        messageId(0),
        nBytesReceived(0),
        port(port),
        messageObserver(messageObserver),
        timeOfLastHello(time(0))
      {
         assert(stoppableProgram);
         assert(messageObserver);

         const auto input =   [this]()
                              {
                                 this->InputThread();
                              };

         const auto output =  [this]()
                              {
                                 this->OutputThread();
                              };

         inputThread = std::move(std::thread(input));
         outputThread = std::move(std::thread(output));
      }

      void Join()
      {
         if(inputThread.joinable())
            inputThread.join();

         if(outputThread.joinable())
            outputThread.join();
      }

      void Stop()
      {
         stopped = true;
      }

      void RethrowException()
      {
         if(inputExceptionPointer)
            std::rethrow_exception(inputExceptionPointer);

         if(outputExceptionPointer)
            std::rethrow_exception(outputExceptionPointer);

         if(badAlloc)
            throw std::bad_alloc();
      }

      void AddMessage(const std::string & message)
      {
         assert(&message);

         if(clientSocket)
            messages.Push(message);
      }

      void SendChatMessage(const std::string & chatMessage, const std::string & user)
      {
         assert(&chatMessage);
         assert(&user);

         std::string message;
         message += "chat";
         message += " ";
         message += user;
         message += " ";
         message += chatMessage;

         AddMessage(message);
      }

      void SendCommandMessage(const std::string & user, const std::string & commandDescription, const std::string & actualCommand, uint32_t commandId, bool isFromChat)
      {
         assert(&user);
         assert(&commandDescription);
         assert(&actualCommand);

         std::string message;
         message += "command";
         message += " ";
         message += isFromChat ? "from_chat" : "not_from_chat";
         message += " ";
         message += user != "" ? user : "-";
         message += " ";
         message += std::to_string(commandId);
         message += " ";
         message += std::to_string(commandDescription.length());
         message += " ";
         message += commandDescription;
         message += actualCommand;

         AddMessage(message);
      }

      void SendCommandBeginMessage(uint32_t commandId)
      {
         std::string message;
         message += "command_begin";
         message += " ";
         message += std::to_string(commandId);

         AddMessage(message);
      }

      void SendCommandEndMessage(uint32_t commandId)
      {
         std::string message;
         message += "command_end";
         message += " ";
         message += std::to_string(commandId);

         AddMessage(message);
      }

      void SendVariableMessage(const std::string & variable, const std::string & value)
      {
         assert(&variable);
         assert(&value);

         std::string message;
         message += "variable";
         message += " ";
         message += variable;
         message += " ";
         message += value;

         AddMessage(message);
      }

   private:
      void InputThread()
      {
         InputOutputThread(&UnityThreads::ReceiveMessages, inputLock, outputLock, inputExceptionPointer, inputHasConnected, outputHasConnected);
      }

      void OutputThread()
      {
         InputOutputThread(&UnityThreads::SendMessages, outputLock, inputLock, outputExceptionPointer, outputHasConnected, inputHasConnected);
      }

      template <typename ThreadFunction>
      void InputOutputThread(ThreadFunction threadFunction, std::mutex & lock, std::mutex & otherLock, std::exception_ptr & exceptionPointer, bool & hasConnected, bool & otherHasConnected)
      {
         assert(threadFunction);
         assert(&lock);
         assert(&otherLock);
         assert(&exceptionPointer);
         assert(&hasConnected);
         assert(&otherHasConnected);

         try
         {
            ResetConnection(otherLock, hasConnected, otherHasConnected);

            while(!stopped)
            {
               {
                  std::lock_guard<std::mutex> l1(connectingLock);
                  otherHasConnected = false;
               }

               try
               {
                  std::lock_guard<std::mutex> l2(lock);

                  if(clientSocket)
                     (this->*threadFunction)();
               }
               catch(TcpSocketException &)
               {
                  ResetConnection(otherLock, hasConnected, otherHasConnected);
               }
               catch(BadPackageException &)
               {
                  std::cout << "Bad package." << std::endl;////
                  ResetConnection(otherLock, hasConnected, otherHasConnected);
               }

               Thread::Sleep(10);
            }
         }
         catch(...)
         {
            stoppableProgram->Stop();
            SetExceptionPointer(exceptionPointer);
         }
      }

      void ResetConnection(std::mutex & otherLock, bool & hasConnected, bool & otherHasConnected)
      {
         assert(&otherLock);
         assert(&hasConnected);
         assert(&otherHasConnected);

         std::lock_guard<std::mutex> l1(connectingLock); // Make sure only one of the threads waits for a connection at a time.

         if(otherHasConnected) // Return if the other thread has already connected.
            return;

         std::lock_guard<std::mutex> l2(otherLock); // Make sure we don't change clientSocket while it's being used by the other thread.

         if(stopped)
            return;

         static bool hasBeenConnected = false;
         if(hasBeenConnected)
            std::cout << "Lost connection to Unity program." << std::endl;

         std::cout << "Waiting for Unity program to connect on port " << port << "." << std::endl;

         clientSocket.reset();

         while(!stopped && !clientSocket)
         {
            try
            {
               clientSocket = serverSocket.Accept();
               clientSocket->SetTimeout(2);
               clientSocket->SetNoDelay(true);
               nBytesReceived = 0;

               hasConnected = true;
               hasBeenConnected = true;
            }
            catch(TimedOutTcpSocketException &)
            {
               Thread::Sleep(100);
            }
         }

         if(!stopped)
            std::cout << "Unity program has connected." << std::endl;
      }

      void SetExceptionPointer(std::exception_ptr & exceptionPointer)
      {
         assert(&exceptionPointer);

         try
         {
            exceptionPointer = std::current_exception();
         }
         catch(std::bad_alloc &)
         {
            badAlloc = true;
         }
      }

      void SendMessages()
      {
         assert(clientSocket);

         while(!stopped && !messages.IsEmpty())
         {
            const auto message = messages.Pop();

            SendMessage(message);
         }

         SendHello();
      }

      // Package format
      //
      // Size in bytes     Description
      // -----------------------------
      // 4                 Magic number 0xAAAA5555
      // 4                 Message ID
      // 4                 Package number
      // 1                 Number of bytes of the data field that is used
      // 47                Data
      // 4                 Magic number 0xAA55AA55
      //
      // The size of each package is 64 bytes.
      //
      // For the first package the package number equals one less than the total number of packages.
      // Each succeeding package will have a package number one less than the previous.
      // This means the last package for message will always have a package number of zero.
      void SendMessage(const std::string & message)
      {
         assert(&message);
         assert(clientSocket);
         assert(message.length() <= std::numeric_limits<uint32_t>::max());

         const uint32_t nBytesPerPackage = 47;
         const auto nPackages            = (message.length() + nBytesPerPackage - 1) / nBytesPerPackage;
         uint32_t packageNumber          = nPackages - 1;
         uint32_t nBytesLeftInMessage    = message.length();

         messageId++;

         while(nBytesLeftInMessage != 0)
         {
            const uint32_t magic1                  = 0xaaaa5555;
            const uint32_t magic2                  = 0xaa55aa55;
            const auto nBytesData                  = nBytesLeftInMessage >= nBytesPerPackage ? nBytesPerPackage : nBytesLeftInMessage;
            const auto data                        = &message[message.length() - nBytesLeftInMessage];
            const char dataZeros[nBytesPerPackage] = {0};

            assert(nBytesData == nBytesPerPackage || nBytesLeftInMessage < nBytesPerPackage);

            clientSocket->Send(&magic1,        4);
            clientSocket->Send(&messageId,     4);
            clientSocket->Send(&packageNumber, 4);
            clientSocket->Send(&nBytesData,    1);
            clientSocket->Send(data,           nBytesData);
            clientSocket->Send(dataZeros,      nBytesPerPackage - nBytesData);
            clientSocket->Send(&magic2,        4);

            packageNumber--;
            nBytesLeftInMessage -= nBytesData;
         }
      }

      void SendHello()
      {
         const auto secondsSinceLastHello = difftime(time(0), timeOfLastHello);

         if(secondsSinceLastHello >= 5)
         {
            SendMessage("hello");
            timeOfLastHello = time(0);
         }
      }

      void ReceiveMessages()
      {
         assert(clientSocket);

         uint8_t package[packageSize];

         nBytesReceived = 0;

         while(!stopped && nBytesReceived < packageSize)
            try
            {
               nBytesReceived += clientSocket->Receive(&package[nBytesReceived], packageSize - nBytesReceived);
            }
            catch(TimedOutTcpSocketException &)
            {
               Thread::Sleep(10);
            }

            packageAssembler.AddPackage(package);

            if(packageAssembler.FullMessageWasReceived())
            {
               if(packageAssembler.GetMessage() != "hello")
                  messageObserver->NewMessage(false, "", packageAssembler.GetMessage());
            }
      }

      volatile bool stopped;
      std::thread inputThread;
      std::thread outputThread;
      std::mutex inputLock;
      std::mutex outputLock;
      bool inputHasConnected;
      bool outputHasConnected;
      std::mutex connectingLock;
      std::exception_ptr inputExceptionPointer;
      std::exception_ptr outputExceptionPointer;
      bool badAlloc;
      TcpServerSocket serverSocket;
      Stoppable * const stoppableProgram;
      std::shared_ptr<TcpSocket> clientSocket;
      ThreadSafeQueue<std::string> messages;
      uint32_t messageId;
      uint32_t nBytesReceived;
      uint16_t port;
      const uint32_t packageSize = 64;
      PackageAssembler packageAssembler;
      MessageObserver * const messageObserver;
      time_t timeOfLastHello;
};

#endif
