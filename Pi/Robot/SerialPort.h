#ifndef SerialPort_h
#define SerialPort_h

#include "Posix.h"
#include "Exception.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string>
#include <sstream>
#include <exception>
#include <cassert>
#include <limits>
#include <cstdint>

class SerialPortException :
   public Exception
{
   public:
      enum Type
      {
         FailedToOpen,
         FailedToGetAttributes,
         FailedToSetAttributes,
         FailedToSetBaudRate,
         FailedToRead,
         FailedToWrite
      };

      SerialPortException(Type type, const char * name) :
         type(type)
      {
         assert(name);

         try
         {
            this->name = name;
         }
         catch(...) { }
      }

      Type GetType() const noexcept
      {
         return type;
      }

      const char * Message() const noexcept
      {
         switch(type)
         {
            case FailedToOpen:
               return "Failed to open serial port.";

            case FailedToGetAttributes:
               return "Failed to get serial port attributes.";

            case FailedToSetAttributes:
               return "Failed to set serial port attributes.";

            case FailedToSetBaudRate:
               return "Failed to set serial port baud rate.";

            case FailedToRead:
               return "Failed to read from serial port.";

            case FailedToWrite:
               return "Failed to write to serial port.";

            default:
               assert(false);
               return "";
         }
      }

      std::string ExtendedMessage() const
      {
         std::stringstream strWhat;

         strWhat << "Serial port: " << name << std::endl;
         strWhat << "errno: " << ErrnoToString();

         return strWhat.str().c_str();
      }

   private:
      Type type;
      std::string name;
};

class SerialPort
{
   public:
      SerialPort(const char * name, uint32_t baudRate) :
         name(name),
         port(-1),
         oldAttributes({0}),
         oldAttributesHasBeenSaved(false)
      {
         assert(name);

         try
         {
            Connect();
            SetUpSerialPort(baudRate);
         }
         catch(...)
         {
            Close();
            throw;
         }
      }

      ~SerialPort() noexcept
      {
         Close();
      }

      void Write(const void * data, size_t nBytes)
      {
         assert(data);
         assert(port != -1);

         if(write(port, data, nBytes) == -1)
            Throw(SerialPortException::FailedToWrite);
      }

      size_t Read(void * data, size_t nBytes)
      {
         assert(data);
         assert(port != -1);
         assert(nBytes <= (size_t)std::numeric_limits<ssize_t>::max());

         const auto nBytesRead = read(port, data, nBytes);

         if(nBytesRead == -1)
            Throw(SerialPortException::FailedToRead);

         return nBytesRead;
      }

      void Send(const void * data, size_t nBytes)
      {
         assert(data);

         Write(data, nBytes);
      }

      size_t Receive(void * data, size_t nBytes)
      {
         assert(data);

         return Read(data, nBytes);
      }

      SerialPort(const SerialPort &) = delete;
      SerialPort(SerialPort &&) = default;
      SerialPort & operator=(const SerialPort &) = delete;
      SerialPort & operator=(SerialPort &&) = default;

   private:
      void Connect()
      {
         port = open(name.c_str(), O_RDWR | O_NOCTTY);

         if(port == -1)
            Throw(SerialPortException::FailedToOpen);
      }

      void SetUpSerialPort(uint32_t baudRate)
      {
         assert(port != -1);

         SaveOldAttributes();

         termios newAttributes = oldAttributes;

         if(cfsetispeed(&newAttributes, ToBaudRate(baudRate)) == -1 ||
            cfsetospeed(&newAttributes, ToBaudRate(baudRate)) == -1)
            Throw(SerialPortException::FailedToSetBaudRate);

         cfmakeraw(&newAttributes);
         newAttributes.c_cflag &= ~(CSTOPB/* | CRTSCTS*/);
         newAttributes.c_cflag |= CLOCAL | CREAD;
         newAttributes.c_cc[VTIME] = 1; // Wait 0.1 second after the last character was received
         newAttributes.c_cc[VMIN] = 1; // Read at least one character per call

         if(tcsetattr(port, TCSANOW, &oldAttributes) == -1)
            Throw(SerialPortException::FailedToSetAttributes);
      }

      void SaveOldAttributes()
      {
         assert(port != -1);

         if(tcgetattr(port, &oldAttributes) == -1)
            Throw(SerialPortException::FailedToGetAttributes);

         oldAttributesHasBeenSaved = true;
      }

      int ToBaudRate(uint32_t baudRate) const noexcept
      {
         switch(baudRate)
         {
            case 50:       return B50;
            case 75:       return B75;
            case 110:      return B110;
            case 134:      return B134;
            case 150:      return B150;
            case 200:      return B200;
            case 300:      return B300;
            case 600:      return B600;
            case 1200:     return B1200;
            case 1800:     return B1800;
            case 2400:     return B2400;
            case 4800:     return B4800;
            case 9600:     return B9600;
            case 19200:    return B19200;
            case 38400:    return B38400;
            case 57600:    return B57600;
            case 115200:   return B115200;
            case 230400:   return B230400;
            case 460800:   return B460800;
            case 500000:   return B500000;
            case 576000:   return B576000;
            case 921600:   return B921600;
            case 1000000:  return B1000000;
            case 1152000:  return B1152000;
            case 1500000:  return B1500000;
            case 2000000:  return B2000000;
            case 2500000:  return B2500000;
            case 3000000:  return B3000000;
            case 3500000:  return B3500000;
            case 4000000:  return B4000000;

            default:
               Throw(SerialPortException::FailedToSetBaudRate);
         }

         return 0;
      }

      void Close() noexcept
      {
         if(oldAttributesHasBeenSaved)
         {
            // Attempt to restore previous attributes but ignore any errors
            const auto result = tcsetattr(port, TCSANOW, &oldAttributes);
            assert(result == 0);
         }

         if(port != -1)
         {
            const auto result = close(port);
            assert(result == 0);
         }
      }

      void Throw(SerialPortException::Type type) const
      {
         throw SerialPortException(type, name.c_str());
      }

      const std::string name;
      int port;
      termios oldAttributes;
      bool oldAttributesHasBeenSaved;
};

#endif
