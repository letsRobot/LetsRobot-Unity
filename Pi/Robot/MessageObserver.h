#ifndef MessageObserver_h
#define MessageObserver_h

class MessageObserver
{
   public:
      virtual ~MessageObserver() noexcept { }
      virtual void NewMessage(bool fromChat, const std::string & user, const std::string & message) = 0;
};


#endif
