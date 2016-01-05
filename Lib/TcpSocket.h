// Needs a bit of cleaning up

#ifndef TcpSocket_h
#define TcpSocket_h

#include "Exception.h"
#include "Posix.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sstream>
#include <memory>
#include <exception>
#include <cassert>
#include <limits>
#include <cstdint>

class TcpSocketException :
   public Exception
{
   public:
      enum Type
      {
         FailedToResolveServerName,
         FailedToCreate,
         FailedToSetTimeout,
         FailedToSetNoDelay,
         FailedToConnect,
         FailedToReceive,
         FailedToSend,
         FailedToBind,
         FailedToListen,
         FailedToAccept,
         TimedOut
      };

      enum Reason
      {
         None,
         ConnectionWasClosed,
         Reason_TimedOut
      };

      TcpSocketException(Type type, Reason reason, int errorCode) :
         type(type),
         reason(reason),
         errorCode(errorCode)
      { }

      const char * Message() const noexcept
      {
         switch(type)
         {
            case FailedToResolveServerName:
               return "Failed to resolve server name.";

            case FailedToCreate:
               return "Failed to create socket.";

            case FailedToSetTimeout:
               return "Failed to set timeout on socket.";

            case FailedToSetNoDelay:
               return "Failed to set no delay on socket.";

            case FailedToConnect:
               return "Socket failed to connect.";

            case FailedToReceive:
               return "Socket failed to receive data.";

            case FailedToSend:
               return "Socket failed to send data.";

            case FailedToBind:
               return "Failed to bind socket.";

            case FailedToListen:
               return "Socket failed to listen.";

            case FailedToAccept:
               return "Socket failed to accept.";

            case TimedOut:
               return "Socket operation timed out.";

            default:
               assert(false);
               return "";
         }
      }

      virtual std::string ExtendedMessage() const
      {
         std::stringstream str;

         switch(reason)
         {
            case None:
               break;

            case ConnectionWasClosed:
               str << "Connection was closed." << std::endl;
               break;

            case Reason_TimedOut:
               str << "Timed out." << std::endl;
               break;
         }

         switch(type)
         {
            case FailedToResolveServerName:
               str << "Error code = " << NetdbErrorToString(errorCode);
               break;

            case FailedToCreate:
            case FailedToConnect:
            case FailedToReceive:
            case FailedToSend:
            case FailedToSetTimeout:
            case FailedToSetNoDelay:
            case FailedToBind:
            case FailedToListen:
            case FailedToAccept:
               if(reason != ConnectionWasClosed)
                  str << "errno = " << ErrnoToString();
               break;

            default:
               assert(false);
               return "";
         }

         return str.str();
      }

   private:
      Type type;
      Reason reason;
      int errorCode;
};

class TimedOutTcpSocketException :
      public TcpSocketException
{
   public:
      TimedOutTcpSocketException() : TcpSocketException(TimedOut, None, 0)
      { }
};

class ScopedPosixSocket
{
   public:
      ScopedPosixSocket() noexcept : s(-1)
      { }

      ScopedPosixSocket(int s) noexcept : s(s)
      { }

      ~ScopedPosixSocket() noexcept
      {
         Reset(-1);
      }

      void Reset(int s) noexcept
      {
         if(this->s != -1)
            close(this->s);

         this->s = s;
      }

      int Release() noexcept
      {
         const auto oldS = s;
         s = -1;

         return oldS;
      }

      operator int() const noexcept
      {
         return s;
      }

      ScopedPosixSocket(const ScopedPosixSocket &) = delete;
      ScopedPosixSocket(ScopedPosixSocket &&) = default;
      ScopedPosixSocket & operator=(const ScopedPosixSocket &) = delete;
      ScopedPosixSocket & operator=(ScopedPosixSocket &&) = default;

   private:
      int s;
};

void TcpThrow(TcpSocketException::Type type, int errorCode = 0);
void TcpThrow(TcpSocketException::Type type, TcpSocketException::Reason reason, int errorCode = 0);

class TcpSocket
{
   public:
      TcpSocket() { }
      virtual ~TcpSocket() noexcept { }

      virtual size_t Receive(void * buffer, size_t nBytes) = 0;
      virtual void Send(const void * buffer, size_t nBytes) = 0;
      virtual void SetTimeout(uint32_t seconds) = 0;
      virtual void SetNoDelay(bool noDelay) = 0;

      TcpSocket(const TcpSocket &) = delete;
      TcpSocket(TcpSocket &&) = delete;
      TcpSocket & operator=(const TcpSocket &) = delete;
      TcpSocket & operator=(TcpSocket &&) = delete;
};

class PosixTcpSocket : public TcpSocket
{
   public:
      PosixTcpSocket(int tcpSocket) : tcpSocket(tcpSocket)
      { }

      virtual ~PosixTcpSocket() noexcept { }

      size_t Receive(void * buffer, size_t nBytes)
      {
         assert(buffer);

         if(nBytes > (size_t)std::numeric_limits<int>::max())
            nBytes = std::numeric_limits<int>::max();

         const auto resultRecv = recv(tcpSocket, buffer, nBytes, 0);

         if(resultRecv == -1)
         {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
               throw TimedOutTcpSocketException();

            TcpThrow(TcpSocketException::FailedToReceive);
         }

         if(resultRecv == 0 && nBytes != 0)
            TcpThrow(TcpSocketException::FailedToReceive, TcpSocketException::ConnectionWasClosed);

         return resultRecv;
      }

      void Send(const void * buffer, size_t nBytes)
      {
         assert(buffer);

         size_t nBytesSent = 0;
         while(nBytesSent != nBytes)
         {
            const auto resultSend = send(tcpSocket, (char *)buffer + nBytesSent, nBytes - nBytesSent, 0);

            if(resultSend == -1 || (nBytes != 0 && resultSend == 0))
               TcpThrow(TcpSocketException::FailedToSend);

            nBytesSent += resultSend;
         }
      }

      void SetTimeout(uint32_t seconds)
      {
         timeval timeout;
         timeout.tv_sec = seconds;
         timeout.tv_usec = 0;

         if(setsockopt(tcpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1 ||
            setsockopt(tcpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
            TcpThrow(TcpSocketException::FailedToSetTimeout);
      }

      void SetNoDelay(bool noDelay)
      {
         int noDelayValue = 1;

         if(setsockopt(tcpSocket, IPPROTO_TCP, TCP_NODELAY, &noDelayValue, sizeof(noDelayValue)) == -1)
            TcpThrow(TcpSocketException::FailedToSetNoDelay);
      }

      PosixTcpSocket(const PosixTcpSocket &) = delete;
      PosixTcpSocket(PosixTcpSocket &&) = default;
      PosixTcpSocket & operator=(const PosixTcpSocket &) = delete;
      PosixTcpSocket & operator=(PosixTcpSocket &&) = default;

   private:
      ScopedPosixSocket tcpSocket;
};

class TcpServerSocket
{
   public:
      TcpServerSocket(uint16_t port, bool nonblocking = false)
      {
         serverSocket.Reset(socket(AF_INET, SOCK_STREAM, 0));

         if(serverSocket == -1)
            TcpThrow(TcpSocketException::FailedToCreate);

         sockaddr_in serverAddress = {0};
         serverAddress.sin_family = AF_INET;
         serverAddress.sin_addr.s_addr = INADDR_ANY;
         serverAddress.sin_port = htons(port);

        int enable = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

         if(bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
            TcpThrow(TcpSocketException::FailedToBind);

         if(listen(serverSocket, 1) == -1)
            TcpThrow(TcpSocketException::FailedToListen);
      }

      std::shared_ptr<TcpSocket> Accept()
      {
         sockaddr_in clientAddress = {0};
         socklen_t clientAddressSize = sizeof(clientAddress);

         if(!HasWaitingConnection())
            throw TimedOutTcpSocketException();

         ScopedPosixSocket clientSocket;
         clientSocket.Reset(accept(serverSocket, (sockaddr *)&clientAddress, &clientAddressSize));

         if(clientSocket == -1)
            TcpThrow(TcpSocketException::FailedToAccept);

         auto returnSocket = std::shared_ptr<TcpSocket>(new PosixTcpSocket(clientSocket));
         clientSocket.Release();

         return returnSocket;
      }

   private:
      bool HasWaitingConnection()
      {
         fd_set fdSet;
         FD_ZERO(&fdSet);
         FD_SET(serverSocket, &fdSet);
         timeval timeout = {0};

         if(select(serverSocket + 1, &fdSet, 0, 0, &timeout) > 0)
            return true;

         return false;
      }

      ScopedPosixSocket serverSocket;
};

class TcpClientSocket : public PosixTcpSocket
{
   public:
      TcpClientSocket(const char * server, uint16_t port) : PosixTcpSocket(ResolveNameAndConnect(server, port))
      {
         assert(server);
      }

      TcpClientSocket(const TcpClientSocket &) = delete;
      TcpClientSocket(TcpClientSocket &&) = default;
      TcpClientSocket & operator=(const TcpClientSocket &) = delete;
      TcpClientSocket & operator=(TcpClientSocket &&) = default;

   private:
      int ResolveNameAndConnect(const char * server, uint16_t port)
      {
         auto addrinfos = ResolveName(server, port);
         auto tcpSocket = Connect(addrinfos); // Connect to the first of the found addresses
         freeaddrinfo(addrinfos);
         return tcpSocket;
      }

      addrinfo * ResolveName(const char * server, uint16_t port)
      {
         addrinfo hints = {0};
         hints.ai_family = AF_INET;
         hints.ai_socktype = SOCK_STREAM;

         std::stringstream strPort;
         strPort << port;

         addrinfo * addrinfos = 0;
         int resultGetaddrinfo = 0;
         if((resultGetaddrinfo = getaddrinfo(server, strPort.str().c_str(), &hints, &addrinfos)))
            TcpThrow(TcpSocketException::FailedToResolveServerName, resultGetaddrinfo);

         return addrinfos;
      }

      int Connect(addrinfo * address)
      {
         assert(address);

         ScopedPosixSocket tcpSocket(socket(AF_INET, SOCK_STREAM, 0));

         if(tcpSocket == -1)
         {
            freeaddrinfo(address);
            TcpThrow(TcpSocketException::FailedToCreate);
         }

         if(connect(tcpSocket, address->ai_addr, address->ai_addrlen) == -1)
         {
            freeaddrinfo(address);
            TcpThrow(TcpSocketException::FailedToConnect);
         }

         return tcpSocket.Release();
      }
};

#endif

