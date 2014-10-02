#ifndef RobotProgram_h
#define RobotProgram_h

#include "IrcThread.h"
#include "StandardInputThread.h"
#include "CommandDescriptions.h"
#include "Users.h"
#include "Robot.h"
#include "MessageDispatcher.h"
#include "ThreadSafeQueue.h"
#include "RobotSettings.h"
#include "Tokenizer.h"
#include "ErrorObserver.h"
#include <string>
#include <cctype>
#include <cassert>
#include <utility>

class RobotProgram
   : private ErrorObserver
{
   public:
      RobotProgram()
         : result(0)
      {
         try
         {
            Run();

            std::cout << "Done." << std::endl;
         }
         catch(std::bad_alloc &)
         {
            Fail("Out of memory.");
         }
         catch(std::exception & e)
         {
            Fail(e.what());
         }
         catch(const char * errorMessage)
         {
            Fail(errorMessage);
         }
         catch(...)
         {
            Fail("Unknown error.");
         }
      }

      int GetResult() const
      {
         return result;
      }

   private:
      // Each of the following is run in it's own thread:
      // - Communications with the IRC server.
      // - Standard input.
      // - The parsing of messages and execution of commands.
      void Run()
      {
         RobotSettings settings("Settings.txt");
         CommandDescriptions commandDescriptions("Commands.txt");
         Users users("Users.txt");

         const auto serialPort = settings.GetString("serial_port");
         const auto baudRate = settings.GetInteger("baud_rate");
         std::cout << "Connecting to robot using " << serialPort << " at " << baudRate << " baud." << std::endl;
         Robot robot(serialPort, baudRate, settings.GetBoolean("show_robot"), settings.GetBoolean("show_robot_debug"));

         messageDispatcher.reset(new MessageDispatcher(&commandDescriptions, &users, settings.GetBoolean("show_chat")));

         ircThread.reset(new IrcThread(settings.GetString("irc_server"),
                                             settings.GetInteger("irc_port"),
                                             settings.GetString("irc_channel"),
                                             settings.GetString("irc_username"),
                                             settings.GetString("irc_password"),
                                             MessageDispatcher::chatId,
                                             messageDispatcher.get(),
                                             this));

         CommandExecuter commandExecuter(&robot, ircThread.get(), settings.GetBoolean("show_commands"));
         messageDispatcher->SetCommandExecuter(&commandExecuter);

         inputThread.reset(new StandardInputThread(MessageDispatcher::inputId, messageDispatcher.get(), this));

         messageDispatcher->DispatchMessages();

         // Tell the threads to stop
         ircThread->Stop();
         inputThread->Stop();

         // and wait for them to finish.
         ircThread->Join();
         inputThread->Join();

         // If any of the threads exited because of an exception rethrow it here.
         ircThread->RethrowException();
         inputThread->RethrowException();
      }

      void Fail(const char * errorMessage = "")
      {
         assert(errorMessage);

         result = 1;

         std::cout << "A nonrecoverable error has occurred." << std::endl;

         if(*errorMessage)
            std::cout << errorMessage << std::endl;
      }

      void ReportFatalError()
      {
         ircThread->Stop();
         inputThread->Stop();
         messageDispatcher->Stop();
      }

      std::unique_ptr<MessageDispatcher> messageDispatcher;
      std::unique_ptr<IrcThread> ircThread;
      std::unique_ptr<StandardInputThread> inputThread;
      int result;
};

#endif
