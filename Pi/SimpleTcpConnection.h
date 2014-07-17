#ifndef SimpleTcpConnection_h
#define SimpleTcpConnection_h

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <cassert>

class SimpleTcpConnection
{
   public:
      SimpleTcpConnection(const char * server, uint16_t port) : s(-1)
      {
         s = socket(AF_INET, SOCK_STREAM, 0);

         if(s < 0)
            throw "Failed to create socket.";

         // Look up address
         addrinfo * addrinfos = 0;
         addrinfo hints = {0};
         hints.ai_family = AF_INET;
         hints.ai_socktype = SOCK_STREAM;

         char strPort[6];
         sprintf(strPort, "%i", (int)port);
         if(getaddrinfo(server, strPort, &hints, &addrinfos) != 0 && !addrinfos)
         {
            Close();
            throw "Failed to look up address.";
         }

         // Connect
         if(connect(s, addrinfos->ai_addr, addrinfos->ai_addrlen) != 0)
         {
            freeaddrinfo(addrinfos);
            Close();
            throw "Failed to connect socket.";
         }

         freeaddrinfo(addrinfos);
      }

      ~SimpleTcpConnection()
      {
         Close();
      }

      void Send(const void * buffer, size_t nBytes)
      {
         if(send(s, buffer, nBytes, 0) == -1)
            throw "Failed to send data.";
      }

      void Send(const std::string & str)
      {
         Send(str.c_str(), str.length());
      }

      size_t Receive(void * buffer, size_t nBytes)
      {
         const auto nBytesReceived = recv(s, buffer, nBytes, 0);

         if(nBytesReceived < 0)
            throw "Failed to receive data.";
         if(nBytesReceived == 0)
            throw "Failed to receive data. Connection was closed.";

         return nBytesReceived;
      }

   private:
      void Close()
      {
         if(s != -1)
            close(s);
      }

      int s;
};

#endif
