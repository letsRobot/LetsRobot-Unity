#ifndef Exception_h
#define Exception_h

#include <string>
#include <exception>

class Exception :
   public std::exception
{
   public:
      const char * what() const noexcept
      {
         return Message();
      }

      virtual const char * Message() const noexcept
      {
         return "";
      }

      virtual std::string ExtendedMessage() const
      {
         return "";
      }
};

#endif

