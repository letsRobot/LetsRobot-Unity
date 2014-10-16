#ifndef BufferedInput_h
#define BufferedInput_h

#include <string>
#include <cstring>
#include <memory>
#include <utility>
#include <cassert>

class NoTimeOutException { };

template <typename Connection, typename TimedOutException = NoTimeOutException>
class BufferedInput
{
   public:
      BufferedInput(Connection connection, size_t bufferSize) noexcept :
         connection(connection),
         bufferSize(bufferSize),
         buffer(new char[bufferSize]),
         iBuffer(0),
         endBuffer(0),
         stopped(false)
      {
         assert(connection);
         assert(bufferSize > 0);
      }

      // Reads until \n and removes the \r before \n if there is any
      std::string ReadLine()
      {
         std::string line;

         char c;
         while((c = ReadChar()) == '\r');

         while(c != '\n')
         {
            line += c;
            c = ReadChar();
         }

         if(line.back() == '\r')
            line.pop_back();

         return std::move(line);
      }

      char PeekChar()
      {
         FillBufferIfNecesarry();
         return buffer.get()[iBuffer];
      }

      char ReadChar()
      {
         FillBufferIfNecesarry();
         return buffer.get()[iBuffer++];
      }

      void Stop()
      {
         stopped = true;
      }

   private:
      void FillBufferIfNecesarry()
      {
         if(iBuffer == endBuffer)
         {
            while(!stopped)
            {
               try
               {
                  endBuffer = connection->Receive(buffer.get(), bufferSize);
                  break;
               }
               catch(TimedOutException &)
               { }
            }

            iBuffer = 0;
         }
      }

      Connection connection;
      size_t bufferSize;
      std::unique_ptr<char[]> buffer;
      size_t iBuffer;
      size_t endBuffer;
      volatile bool stopped;
};

#endif
