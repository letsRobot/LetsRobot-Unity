#ifndef Unity_h
#define Unity_h

#include <string>

class Unity
{
   public:
      virtual ~Unity() { }

      virtual void SendChatMessage(const std::string & /*chatMessage*/, const std::string & /*user*/) { }
      virtual void SendVariableMessage(const std::string & /*variable*/, const std::string /*value*/) { }
};

#endif
