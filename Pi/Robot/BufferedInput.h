#ifndef BufferedInput_h
#define BufferedInput_h

//Read lines from an input like a serial port or an internet socket, and store the characters in a buffer instead of reading them one at a time.
//This is more effecient than reading characters one at a time since serial port uses a non-trivial amount of time before it times out.

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

      std::string ReadLine()
      {
         std::string line;

         char c;
         while((c = ReadChar()) == '\r');

         while(c != '\n' && c)
         {
            line += c;
            c = ReadChar();
         }

         while(line.length() > 0 && line.back() == '\r')
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

                  if(endBuffer != 0)
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
