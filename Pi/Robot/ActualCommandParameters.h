#ifndef ActualCommandParameters_h
#define ActualCommandParameters_h

#include <string>
#include <cstdint> //This header defines a set of integral type aliases with specific width requirements, 
//along with macros specifying their limits and macro functions to create values of these types.

class ActualCommandParameters //Used in CommandDescriptions.h
{
   public:
      virtual ~ActualCommandParameters() noexcept { }

      virtual std::string GetWord(size_t iParameter) const = 0;
      virtual std::string GetString(size_t iParameter) const = 0;
      virtual int32_t GetInteger(size_t iParameter) const = 0;
};

#endif
