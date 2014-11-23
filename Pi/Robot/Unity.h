#ifndef Unity_h
#define Unity_h

#include <string>

class Unity
{
   public:
      virtual ~Unity() { }

      virtual void SendChatMessage(const std::string & chatMessage, const std::string & user) = 0;
      virtual void SendVariableMessage(const std::string & variable, const std::string & value) = 0;
};

class UnityDummy : public Unity
{
   void SendChatMessage(const std::string &, const std::string &)
   { }

   void SendVariableMessage(const std::string &, const std::string &)
   { }
};

#endif
