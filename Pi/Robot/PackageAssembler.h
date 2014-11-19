#ifndef PackageAssembler_h
#define PackageAssembler_h

#include <string>
#include <cstring>
#include <cstdint>
#include <limits>
#include <utility>

class BadPackageException
{ };

class PackageAssembler
{
   public:
      PackageAssembler()
         : lastMessageId(std::numeric_limits<uint32_t>::max()),
           lastPackageNumber(0),
           fullMessageReceived(false),
           package(0),
           iByte(0)
      { }

      void AddPackage(uint8_t * package)
      {
         assert(package);

         SetPackage(package);

         const auto magic1        = Read<uint32_t>();
         const auto messageId     = Read<uint32_t>();
         const auto packageNumber = Read<uint32_t>();
         const auto nBytesOfData  = Read<uint8_t>();
         const auto data          = Read(47);
         const auto magic2        = Read<uint32_t>();

         if(magic1 != 0xaaaa5555                              ||
            magic2 != 0xaa55aa55                              ||
            nBytesOfData > 47                                 ||
            (messageId == lastMessageId && packageNumber != lastPackageNumber - 1))
         {
            throw BadPackageException();
         }

         if(messageId != lastMessageId)
            message.clear();

         message += data;

         fullMessageReceived = packageNumber == 0;

         lastMessageId     = messageId;
         lastPackageNumber = packageNumber;

         if(fullMessageReceived)
            message.resize(strlen(message.c_str()));
      }

      bool FullMessageWasReceived() const
      {
         return fullMessageReceived;
      }

      const std::string & GetMessage() const
      {
         assert(fullMessageReceived);

         return message;
      }

   private:
      void SetPackage(uint8_t * package)
      {
         this->package = package;
         iByte = 0;
      }

      template <typename T>
      T Read()
      {
         assert(iByte + sizeof(T) <= 64);

         const auto value = *(T *)(&package[iByte]);
         iByte += sizeof(T);
         return value;
      }

      std::string Read(size_t nBytes)
      {
         assert(iByte + nBytes <= 64);

         std::string buffer((char *)&package[iByte], nBytes);
         iByte += nBytes;

         return std::move(buffer);
      }

      uint32_t lastMessageId;
      uint32_t lastPackageNumber;
      bool fullMessageReceived;
      std::string message;
      uint8_t * package;
      size_t iByte;
};

#endif
