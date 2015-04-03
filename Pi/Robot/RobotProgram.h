#ifndef RobotProgram_h
#define RobotProgram_h

#include "IrcThread.h"
#include "StandardInputThread.h"
#include "UnityThreads.h"
#include "CommandExecuterThread.h"
#include "MessageDispatcher.h"
#include "CommandDescriptions.h"
#include "Users.h"
#include "Robot.h"
#include "ThreadSafeQueue.h"
#include "RobotSettings.h"
#include "Tokenizer.h"
#include "Stoppable.h"
#include <mutex>
#include <string>
#include <cctype>
#include <cassert>
#include <utility>

extern MessageObserver * sigIntHandlerMessageObserver;

class RobotProgram
   : private Stoppable
{
   public:
      // This is where we go from main()
      RobotProgram()
         : result(0)
      {
         try
         {
            Run(); // Call member function that actually does things
         }
         // React to different kinds of errors that Run() might throw
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

         std::cout << "Done." << std::endl;
      }

      ~RobotProgram()
      {
         sigIntHandlerMessageObserver = 0;
      }

      int GetResult() const
      {
         return result;
      }

   private:
      // Each of the following is run in it's own thread:
      // - Communications with the IRC server.
      // - Standard input.
      // - The parsing of messages.
      // - The execution of commands.
      // - Communications with the Unity program.
      void Run()
      {
         try
         {
            std::unique_lock<std::mutex> l(stopLock);

            RobotSettings settings("Settings.txt"); // Parse the settings file and store the values
            CommandDescriptions commandDescriptions("Commands.txt"); // Parse the command descriptions file
            Users users("Users.txt"); // Parse the users file and store the privilege levels

            // Connect to robot
            const auto serialPort = settings.GetString("serial_port");
            const auto baudRate = settings.GetInteger("baud_rate");
            std::cout << "Connecting to robot using " << serialPort << " at " << baudRate << " baud." << std::endl;
            Robot robot(serialPort, baudRate, settings.GetBoolean("show_robot"), settings.GetBoolean("show_robot_debug"));

            messageDispatcher.reset(new MessageDispatcher(&commandDescriptions, &users, settings.GetBoolean("show_chat")));

            sigIntHandlerMessageObserver = messageDispatcher.get();

            // Create and run all the different threads; look at their respective files to see what
            // they are actually doing. Each one is essentially a mini-program with its own loop.
            ircThread.reset(new IrcThread(settings.GetString("irc_server"),
                                          settings.GetInteger("irc_port"),
                                          settings.GetString("irc_channel"),
                                          settings.GetString("irc_username"),
                                          settings.GetString("irc_password"),
                                          messageDispatcher.get(),
                                          this));

            commandExecuterThread.reset(new CommandExecuterThread(&robot, ircThread.get(), settings.GetBoolean("show_commands"), messageDispatcher.get(), this));

            messageDispatcher->SetCommandExecuterThread(commandExecuterThread.get());

            inputThread.reset(new StandardInputThread(messageDispatcher.get(), this));

            unityThreads.reset(new UnityThreads(settings.GetInteger("unity_local_port"), this, messageDispatcher.get()));

            messageDispatcher->SetUnityThreads(unityThreads.get());
            commandExecuterThread->SetUnityThreads(unityThreads.get());

            l.unlock();

            messageDispatcher->DispatchMessages();

            // Wait for all of the threads to complete, and then quit
            StopAndJoinThreads();

            // If any of the threads exited because of an exception rethrow it here.
            RethrowException();
         }
         catch(...)
         {
            StopAndJoinThreads();
            throw;
         }
      }

      void Fail(const char * errorMessage = "")
      {
         assert(errorMessage);

         result = 1;

         std::cout << "*** A nonrecoverable error has occurred. ***" << std::endl;

         if(*errorMessage)
            std::cout << errorMessage << std::endl;
      }

      void StopAndJoinThreads()
      {
         Stop();
         Join();
      }

      void Stop()
      {
         std::lock_guard<std::mutex> l(stopLock);

         if(ircThread)
            ircThread->Stop();

         if(inputThread)
            inputThread->Stop();

         if(unityThreads)
            unityThreads->Stop();

         if(commandExecuterThread)
            commandExecuterThread->Stop();

         if(messageDispatcher)
            messageDispatcher->Stop();
      }

      void Join()
      {
         if(ircThread)
            ircThread->Join();

         if(inputThread)
            inputThread->Join();

         if(unityThreads)
            unityThreads->Join();

         if(commandExecuterThread)
            commandExecuterThread->Join();
      }

      void RethrowException()
      {
         if(ircThread)
            ircThread->RethrowException();

         if(inputThread)
            inputThread->RethrowException();

         if(unityThreads)
            unityThreads->RethrowException();

         if(commandExecuterThread)
            commandExecuterThread->RethrowException();
      }

      std::unique_ptr<MessageDispatcher> messageDispatcher;
      std::unique_ptr<IrcThread> ircThread;
      std::unique_ptr<StandardInputThread> inputThread;
      std::unique_ptr<UnityThreads> unityThreads;
      std::unique_ptr<CommandExecuterThread> commandExecuterThread;
      int result;
      std::mutex stopLock;
};

#endif
