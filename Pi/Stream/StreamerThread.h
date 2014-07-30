#ifndef StreamerThread_h
#define StreamerThread_h

#include <TcpSocket.h>
#include <Thread.h>
#include <Posix.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <memory>
#include <exception>
#include <limits>

class StreamerThread :
      public Thread
{
   public:
      StreamerThread(const char * filename, uint16_t port) :
         filename(filename),
         port(port),
         serverSocket(port),
         stopped(false),
         fileChanged(false),
         lastInode(0),
         firstTimeWaitingForFileChange(true),
         currentBufferSize(0)
      {
         Start();
      }

      ~StreamerThread() noexcept
      {
         Stop();
         Thread::Join();
      }

      void Stop()
      {
         stopped = true;
      }

      void ThrowException()
      {
         if(exceptionPointer)
            std::rethrow_exception(exceptionPointer);
      }

   private:
      void Run()
      {
         try
         {
            while(!stopped)
            {
               if(!clientSocket)
                  std::cout << "Listening on port " << port << "." << std::endl;

               fileChanged = false;

               while(!stopped && !(clientSocket && fileChanged))
               {
                  if(!clientSocket)
                     CheckForConnection();

                  if(!fileChanged)
                     CheckIfFileChanged();

                  Thread::Sleep(10);
               }

               if(stopped)
                  return;

               if(!PrepareBuffer())
               {
                  std::cerr << "Failed to read file." << std::endl;
                  continue;
               }

               if(stopped)
                  return;

               SendFile();
            }
         }
         catch(...)
         {
            exceptionPointer = std::current_exception();
         }
      }

      // In reality we check if the inode associated with the filename has changed
      void CheckIfFileChanged()
      {
         struct stat s;

         if(stat(filename.c_str(), &s) == -1)
         {
            std::cerr << "File was not found." << std::endl;
            Thread::Sleep(1000);
            return;
         }

         if(firstTimeWaitingForFileChange)
         {
            lastInode = s.st_ino;
            firstTimeWaitingForFileChange = false;
         }

         if(s.st_ino != lastInode)
         {
            lastInode = s.st_ino;

            fileChanged = true;
         }
      }

      bool PrepareBuffer()
      {
         ScopedFile file(open(filename.c_str(), O_RDONLY));

         if(file == -1)
            return false;

         const auto size = lseek(file, 0, SEEK_END);
         if(size == -1)
            return false;

         if(size == 0)
            return false;

         if(lseek(file, 0, SEEK_SET) == -1)
            return false;

         if(size > std::numeric_limits<uint32_t>::max())
         {
            std::cerr << "File is too big." << std::endl;
            return false;
         }

         const auto buffer = GetBuffer(size);

         // The data that is sent has the following structure:
         //
         // Offset   Size           Description
         // -------------------------------------------
         // 0        4              Magic number 0x1234
         // 4        4              File size
         // 8        4              Magic number 0x5678
         // 12       4              File size
         // 16       4              Magic number 0xabcd
         // 20       File size      File data

         *(uint32_t *)&buffer[0]  = 0x1234;
         *(uint32_t *)&buffer[4]  = size;
         *(uint32_t *)&buffer[8]  = 0x5678;
         *(uint32_t *)&buffer[12] = size;
         *(uint32_t *)&buffer[16] = 0xabcd;

         if(read(file, buffer + 20, size) == -1)
            return false;

         return true;
      }

      char * GetBuffer(off_t size)
      {
      	const auto newContentSize = size + 20;

      	if(size > currentBufferSize)
         {
            buffer.reset(new char[newContentSize]);
            currentBufferSize = size;
         }

         bufferContentSize = newContentSize;

         return buffer.get();
      }

      void CheckForConnection()
      {
         assert(!clientSocket);

         try
         {
            clientSocket = serverSocket.Accept();
            clientSocket->SetTimeout(10);
            clientSocket->SetNoDelay(true);

            std::cout << "Connected." << std::endl;
         }
         catch(TimedOutTcpSocketException &)
         {
            return;
         }
      }

      void SendFile()
      {
         assert(clientSocket);

         try
         {
            clientSocket->Send(buffer.get(), bufferContentSize);
         }
         catch(TcpSocketException & e)
         {
            ConnectionWasLost();
         }
      }

      void ConnectionWasLost()
      {
         std::cout << "Connection was lost." << std::endl;
         clientSocket.reset();
         firstTimeWaitingForFileChange = true;

      }

      std::string filename;
      uint16_t port;
      TcpServerSocket serverSocket;
      std::shared_ptr<TcpSocket> clientSocket;

      bool stopped;
      bool fileChanged;

      ino_t lastInode;
      bool firstTimeWaitingForFileChange;

      std::unique_ptr<char[]> buffer;
      off_t currentBufferSize;
      off_t bufferContentSize;

      std::exception_ptr exceptionPointer;
};

#endif

