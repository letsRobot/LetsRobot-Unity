#ifndef MessageDispatcher_h
#define MessageDispatcher_h

#include "CommandDescriptions.h"
#include "Users.h"
#include "MessageObserver.h"
#include "ThreadSafeQueue.h"
#include "Tokenizer.h"
#include "CommandExecuterThread.h"
#include "UnityThreads.h"
#include <ctime>
#include <sstream>
#include <utility>
#include <string>
#include <cassert>

struct Message
{
   public:
      Message(const std::string & user, const std::string & originalMessage, bool isFromChat)
         : user(user),
           originalMessage(originalMessage),
           isFromChat(isFromChat)
      {
         assert(&user);
         assert(&originalMessage);
      }

      const std::string & GetUser() const
      {
         return user;
      }

      const std::string & GetMessage() const
      {
         return originalMessage;
      }

      const bool IsFromChat() const
      {
         return isFromChat;
      }

   private:
      const std::string user;
      std::string cleanMessage;
      const std::string originalMessage;
      bool isFromChat;
};

// This class receives messages, parses them, and executes the ones that are commands.
class MessageDispatcher
   : public MessageObserver
{
   public:
      MessageDispatcher(CommandDescriptions * commandDescriptions, Users * users, bool showChat)
         : stopped(false),
           commandDescriptions(commandDescriptions),
           commandExecuter(0),
           unity(0),
           users(users),
           showChat(showChat)
      {
         assert(commandDescriptions);
         assert(users);
      }

      void Stop()
      {
         stopped = true;
      }

      void SetCommandExecuterThread(CommandExecuterThread * commandExecuter)
      {
         assert(commandExecuter);

         commandExecuter->SetShowChat(&showChat);
         this->commandExecuter = commandExecuter;
      }

      void SetUnityThreads(UnityThreads * unity)
      {
         assert(unity);

         this->unity = unity;
      }

      // This function is called by any of the threads that provides the messages whenever they have a new message ready.
      // If the input was entered directly into the program the user parameter is the empty string.
      void NewMessage(bool fromChat, const std::string & user, const std::string & message)
      {
         assert(&user);
         assert(&message);

         if(fromChat && user.empty())
            return;

         if(showChat && fromChat)
            std::cout << "Message from " << user << ": " << message << std::endl;

         messageQueue.Push(Message(user, message, fromChat));
      }

      // Waits for the message queue to become non-empty.
      // Then executes any messages recognized as commands in the queue until it's empty again.
      void DispatchMessages()
      {
         while(!stopped)
         {
            Thread::Sleep(10); // This should really be done with a conditional variable instead.

            while(!stopped && !messageQueue.IsEmpty())
            {
               Message message = messageQueue.Pop();

               const auto actualCommand = ParseMessage(message.GetMessage());
               const bool messageIsCommand = actualCommand.GetCommandDescription();
               const bool executeMessageAsCommand = messageIsCommand && UserMayUseCommand(*actualCommand.GetCommandDescription(), message.GetUser());
               if(!executeMessageAsCommand)
               {
                  if(!message.IsFromChat())
                     std::cout << "Unknown command: " << message.GetMessage() << std::endl;

                  else if(unity)
                     unity->SendChatMessage(message.GetMessage(), message.GetUser());

                  continue;
               }

               if(commandExecuter)
                  commandExecuter->AddCommand(actualCommand, message.GetUser(), message.IsFromChat());
            }
         }
      }

   private:
      ActualCommand ParseMessage(const std::string & message) const
      {
         assert(&message);

         auto actualCommand = ActualCommand(message);
         commandDescriptions->FindCorrespondingCommandDescription(actualCommand);

         return std::move(actualCommand);
      }

      bool UserMayUseCommand(const CommandDescription & commandDescription, const std::string & user) const
      {
         assert(&commandDescription);
         assert(&user);

         if(user.empty()) // Allow any command that was entered directly into the program.
            return true;

         return commandDescription.GetPrivilegeLevel() <= users->GetPrivilegeLevel(user);
      }

      volatile bool stopped;
      CommandDescriptions * const commandDescriptions;
      CommandExecuterThread * commandExecuter;
      UnityThreads * unity;
      ThreadSafeQueue<Message> messageQueue;
      Users * const users;
      bool showChat;
};

#endif
