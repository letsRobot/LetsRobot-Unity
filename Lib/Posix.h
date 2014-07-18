#ifndef Posix_h
#define Posix_h

#include <unistd.h>
#include <string>

std::string ErrnoToString();
std::string NetdbErrorToString(int errorCode);

class ScopedFile
{
   public:
      ScopedFile() :
         file(-1)
      { }

      explicit ScopedFile(int file) :
         file(file)
      { }

      ~ScopedFile()
      {
         Reset();
      }

      void Reset(int file = -1)
      {
         if(this->file != -1)
            close(this->file);

         this->file = file;
      }

      operator int()
      {
         return file;
      }

   private:
      int file;
};

#endif

