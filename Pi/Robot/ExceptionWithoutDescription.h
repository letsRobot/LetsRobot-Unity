#ifndef ExceptionWithoutDescription_h
#define ExceptionWithoutDescription_h

#include <exception>

class ExceptionWithoutDescription
   : public std::exception
{
   public:
      const char * what() const noexcept
      {
         return "";
      }
};

#endif
