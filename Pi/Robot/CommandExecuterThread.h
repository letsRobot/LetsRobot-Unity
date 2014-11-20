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

struct CommandReadyForExecution
{
   const ActualCommand actualCommand;
   const std::string user;
   const uint32_t id;

   CommandReadyForExecution(const ActualCommand & actualCommand, const std::string & user, uint32_t id)
      : actualCommand(actualCommand),
        user(user),
        id(id)
   { }
};

class CommandExecuterThread
   : public Thread
{
   public:
      CommandExecuterThread(Robot * robot, IrcThread * irc, bool showCommands, MessageObserver * messageObserver, Stoppable * stoppableProgram)
         : robot(robot),
           irc(irc),
           showCommands(showCommands),
           showChat(&showChatDummy),
           messageObserver(messageObserver),
           stoppableProgram(stoppableProgram),
           unity(0),
           commandCallUnity(&unityDummy),
           commandId(1)
      {
         assert(robot);
         assert(irc);
         assert(messageObserver);
         assert(stoppableProgram);

         BuildCommandFunctionMap();
         Start();
      }

      void SetUnityThreads(UnityThreads * unity)
      {
         assert(unity);

         this->unity = unity;
         commandCallUnity = unity;
      }

      void SetShowChat(bool * showChat)
      {
         assert(showChat);

         this->showChat = showChat;
      }

      void AddCommand(const ActualCommand & actualCommand, const std::string & user, bool isFromChat)
      {
         assert(&actualCommand);

         commands.Push(CommandReadyForExecution(actualCommand, user, commandId));

         if(unity)
            unity->SendCommandMessage(user, actualCommand.GetCommandDescriptionString(), actualCommand.GetCommandString(), commandId, isFromChat);

         commandId++;
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

               Thread::Sleep(10);
            }
         }
         catch(...)
         {
            stoppableProgram->Stop();
            throw;
         }
      }

      void ExecuteCommand(const CommandReadyForExecution & command)
      {
         assert(&command);

         const auto commandDescription = command.actualCommand.GetCommandDescription()->GetString();

         if(CommandNeedsToCoolDown(command.actualCommand.GetCommandDescription()))
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

         if(iCommandFunction == commandFunctions.end())
            return;

         if(unity)
            unity->SendCommandBeginMessage(command.id);

         iCommandFunction->second(command.actualCommand, *robot, *irc, showCommands, *showChat, *unity, *messageObserver);

         if(unity)
            unity->SendCommandEndMessage(command.id);
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
      MessageObserver * const messageObserver;
      Stoppable * const stoppableProgram;
      std::map<std::string, CommandFunctions::CommandFunctionPointer> commandFunctions;
      std::map<std::string, time_t> timesOfLastCommands;
      ThreadSafeQueue<CommandReadyForExecution> commands;
      UnityThreads * unity;
      Unity unityDummy;
      Unity * commandCallUnity;
      uint32_t commandId;
};

#endif
