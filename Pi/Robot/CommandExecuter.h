#ifndef CommandExecuter_h
#define CommandExecuter_h

#include "IrcThread.h"
#include "CommandDescriptions.h"
#include "Robot.h"
#include "CommandFunctions.h"
#include <map>
#include <cassert>

class CommandExecuter
{
   public:
      CommandExecuter(Robot * robot, IrcThread * irc, bool showCommands)
         : robot(robot),
           irc(irc),
           showCommands(showCommands),
           showChat(&showChatDummy)
      {
         assert(robot);
         assert(irc);

         BuildCommandFunctionMap();
      }

      void SetShowChat(bool * showChat)
      {
         assert(showChat);

         this->showChat = showChat;
      }

      // Returns true after the quit command has been executed.
      bool ExecuteCommand(const ActualCommand & actualCommand, const std::string & message)
      {
         assert(&actualCommand);
         assert(&message);

         const auto commandDescription = actualCommand.GetCommandDescription()->GetString();

         if(CommandNeedsToCoolDown(actualCommand.GetCommandDescription()))
            return false;

         if(showCommands)
            std::cout << "Executing command: " << commandDescription << std::endl;

         if(commandDescription == "/q")
         {
            std::cout << "Quitting." << std::endl;
            return true;
         }

         auto iCommandFunction = commandFunctions.find(commandDescription);

         if(iCommandFunction != commandFunctions.end())
            iCommandFunction->second(actualCommand, *robot, *irc, showCommands, *showChat);

         return false;
      }

   private:
      void BuildCommandFunctionMap()
      {
         auto node = CommandFunctions::First();

         while(node)
         {
            commandFunctions[node->command] = node->function;

            node = node->next;
         }
      }

      bool CommandNeedsToCoolDown(const CommandDescription * commandDescription)
      {
         const auto commandCooldown = commandDescription->GetCooldownTime();
         const auto commandHasCooldown = commandCooldown != 0;

         if(commandHasCooldown)
         {
            const auto iTimeOfLastCommand = timesOfLastCommands.find(commandDescription->GetString());

            if(iTimeOfLastCommand != timesOfLastCommands.end())
            {
               const auto elapsedTime = difftime(time(0), iTimeOfLastCommand->second);

               if(elapsedTime < commandCooldown)
                  return true;
            }
         }

         timesOfLastCommands[commandDescription->GetString()] = time(0);
         return false;
      }

      Robot * const robot;
      IrcThread * irc;
      bool showCommands;
      bool showChatDummy;
      bool * showChat;
      std::map<std::string, CommandFunctions::CommandFunctionPointer> commandFunctions;
      std::map<std::string, time_t> timesOfLastCommands;
};

#endif
