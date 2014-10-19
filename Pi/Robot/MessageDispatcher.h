#ifndef MessageDispatcher_h
#define MessageDispatcher_h

#include "CommandDescriptions.h"
#include "Users.h"
#include "MessageObserver.h"
#include "ThreadSafeQueue.h"
#include "Tokenizer.h"
#include "CommandExecuterThread.h"
#include <ctime>
#include <sstream>
#include <utility>
#include <string>
#include <cassert>

struct Message
{
   public:
      Message(const std::string & user = "", const std::string & originalMessage = "")
         : user(user),
           originalMessage(originalMessage)
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

   private:
      const std::string user;
      std::string cleanMessage;
      const std::string originalMessage;
};

// This class receives messages, parses them, and executes the ones that are commands.
class MessageDispatcher
   : public MessageObserver
{
   public:
      static const uint32_t inputId = 0;
      static const uint32_t chatId  = 1;

      MessageDispatcher(CommandDescriptions * commandDescriptions, Users * users, bool showChat)
         : stopped(false),
           commandDescriptions(commandDescriptions),
           commandExecuter(0),
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

      // This function is called by any of the threads that provides the messages whenever they have a new message ready.
      // If the input was entered directly into the program the user parameter is the empty string.
      void NewMessage(uint32_t id, const std::string & user, const std::string & message)
      {
         assert(id == inputId || id == chatId);
         assert(&user);
         assert(&message);

         if(id == chatId && user.empty())
            return;

         if(showChat && id == chatId)
            std::cout << "Message from " << user << ": " << message << std::endl;

         messageQueue.Push(Message(user, message));
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
               if(!actualCommand.GetCommandDescription())
               {
                  if(message.GetUser().empty())
                     std::cout << "Unknown command: " << message.GetMessage() << std::endl;

                  continue;
               }

               if(!UserMayUseCommand(*actualCommand.GetCommandDescription(), message.GetUser()))
                  continue;

               if(commandExecuter)
                  commandExecuter->AddCommand(actualCommand);
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
      ThreadSafeQueue<Message> messageQueue;
      Users * const users;
      bool showChat;
};

#endif
