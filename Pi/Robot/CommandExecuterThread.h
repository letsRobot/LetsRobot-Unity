#ifndef CommandExecuterThread_h
#define CommandExecuterThread_h

#include "IrcThread.h"
#include "CommandDescriptions.h"
#include "Robot.h"
#include "CommandFunctions.h"
#include "Stoppable.h"
#include "ThreadSafeQueue.h"
#include <Thread.h>
#include <map>
#include <cassert>

class CommandExecuterThread
   : public Thread
{
   public:
      CommandExecuterThread(Robot * robot, IrcThread * irc, bool showCommands, Stoppable * stoppableProgram)
         : robot(robot),
           irc(irc),
           showCommands(showCommands),
           showChat(&showChatDummy),
           stoppableProgram(stoppableProgram)
      {
         assert(robot);
         assert(irc);
         assert(stoppableProgram);

         BuildCommandFunctionMap();
         Start();
      }

      void SetShowChat(bool * showChat)
      {
         assert(showChat);

         this->showChat = showChat;
      }

      void AddCommand(const ActualCommand & actualCommand)
      {
         assert(&actualCommand);

         commands.Push(actualCommand);
      }

   private:
      void Run()
      {
         try
         {
            while(!stopped)
            {
               while(!stopped && !commands.IsEmpty())
               {
                  const auto command = commands.Pop();
                  ExecuteCommand(command);
               }

               Thread::Sleep(100);
            }
         }
         catch(...)
         {
            stoppableProgram->Stop();
            throw;
         }
      }

      void ExecuteCommand(const ActualCommand & actualCommand)
      {
         assert(&actualCommand);

         const auto commandDescription = actualCommand.GetCommandDescription()->GetString();

         if(CommandNeedsToCoolDown(actualCommand.GetCommandDescription()))
            return;

         if(showCommands)
            std::cout << "Executing command: " << commandDescription << std::endl;

         if(commandDescription == "/q")
         {
            std::cout << "Quitting." << std::endl;
            stoppableProgram->Stop();
            return;
         }

         auto iCommandFunction = commandFunctions.find(commandDescription);

         if(iCommandFunction != commandFunctions.end())
            iCommandFunction->second(actualCommand, *robot, *irc, showCommands, *showChat);
      }

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
      Stoppable * const stoppableProgram;
      std::map<std::string, CommandFunctions::CommandFunctionPointer> commandFunctions;
      std::map<std::string, time_t> timesOfLastCommands;
      ThreadSafeQueue<ActualCommand> commands;
};

#endif
