#ifndef IrcThread_h
#define IrcThread_h

#include "IrcClient.h"
#include "MessageObserver.h"
#include "ErrorObserver.h"
#include <Thread.h>
#include <iostream>
#include <string>
#include <cstring>
#include <mutex>
#include <memory>
#include <exception>
#include <cassert>

class IrcThread
   : public Thread,
     public IrcClientObserver
{
   public:
      IrcThread(const char * server, uint16_t port, const char * channel, const char * username, const char * password, uint32_t id, MessageObserver * messageObserver, ErrorObserver * errorObserver)
         : server(server),
           port(port),
           channel(channel),
           username(username),
           password(password),
           id(id),
           messageObserver(messageObserver),
           errorObserver(errorObserver)
      {
         assert(server);
         assert(channel);
         assert(username);
         assert(password);
         assert(messageObserver);
         assert(errorObserver);

         Start();
      }

      ~IrcThread() noexcept
      {
         Stop();
         Join();
      }

      void Stop()
      {
         stopped = true;

         if(irc)
            irc->Stop();
      }

      void SendMessage(const char * message)
      {
         assert(message);

         SendWhisper("", message);
      }

      void SendWhisper(const char * user, const char * message)
      {
         assert(user);
         assert(message);

         std::lock_guard<std::mutex> l(ircLock);
         if(irc)
         {
            try
            {
               if(!*user)
                  irc->SendMessageToChannel(message);
               else
                  irc->SendMessageTo(user, message);
            }
            catch(Exception &) { } // If an exception occurs we ignore it and the message is lost.
         }
      }

      void SendActionMessage(const char * message)
      {
         assert(message);

         // This is specific for Twitch.
         std::string str;
         str += "\001ACTION ";
         str += message;
         str += "\001";

         SendMessage(str.c_str());
      }

   private:
      void Run()
      {
         while(!stopped)
         {
            try
            {
               ResetConnection();
//               SendActionMessage("is online and stuff.");
               ReceiveMessages();
            }
            catch(Exception &e)
            {
               {
                  std::lock_guard<std::mutex> l(ircLock);
                  irc.reset();
               }

               const int nSecondsUntilRetry = 10;

               std::cout << "Connection attempt failed." << std::endl << e.Message() << std::endl;
               std::cout << "Will retry in " << nSecondsUntilRetry << (nSecondsUntilRetry == 1 ? " second." : " seconds.") << std::endl;

               for(int i = 0; i < 10 * nSecondsUntilRetry && !stopped; i++)
                  Thread::Sleep(100);
            }
            catch(...)
            {
               errorObserver->ReportFatalError();
               throw;
            }
         }
      }

      void ResetConnection()
      {
         std::cout << "Connecting to IRC server " << server << ":" << port << " as " << username << "." << std::endl;

         std::lock_guard<std::mutex> l(ircLock);
         irc.reset(new IrcClient(server.c_str(), port, username.c_str(), password.c_str()));
         irc->SetSecondsBetweenMessages(2); // Since Twitch only allows 20 messages per 30 seconds waiting 2 seconds will keep us on the safe side.
         irc->GetConnection().SetTimeout(1);
         irc->SetObserver(this);

         std::cout << "Joining IRC channel " << channel << "." << std::endl;
         irc->JoinChannel(channel.c_str());
      }

      void ReceiveMessages()
      {
         while(!stopped)
            irc->NextMessage();
      }

      void IrcWelcome()
      { }

      void IrcMessage(const char * from, const char * to, const char * message)
      {
         assert(from);
         assert(to);
         assert(message);

         if(to != channel && to != username)
            return;

         messageObserver->NewMessage(id, from, message);
      }

      void IrcNotice(const char * from, const char * to, const char * notice)
      {
         assert(from);
         assert(to);
         assert(notice);

         if(strcmp(notice, "Login unsuccessful") == 0)
            std::cout << "IRC notice: Bad username or password." << std::endl;
      }

      void IrcBadMessage(const char * message)
      { }

      const std::string server;
      uint16_t port;
      const std::string channel;
      const std::string username;
      const std::string password;
      const uint32_t id;
      MessageObserver * const messageObserver;
      ErrorObserver * const errorObserver;
      std::unique_ptr<IrcClient> irc;
      std::mutex ircLock; // This lock is used whenever the irc unique pointer is changed.
};

#endif
