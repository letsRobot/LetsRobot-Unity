#ifndef ActualCommandParameters_h
#define ActualCommandParameters_h

#include <cstdint>

class ActualCommandParameters
{
   public:
      virtual ~ActualCommandParameters() noexcept { }

      virtual const char * GetWord(size_t iParameter) const = 0;
      virtual const char * GetString(size_t iParameter) const = 0;
      virtual int32_t GetInteger(size_t iParameter) const = 0;
};


#endif
