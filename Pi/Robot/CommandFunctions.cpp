#include "CommandFunctions.h"

namespace CommandFunctions
{
   Node *& First()
   {
      static Node * first = 0;
      return first;
   }

   Node *& Last()
   {
      static Node * last = 0;
      return last;
   }
}
