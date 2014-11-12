#ifndef Robot_h
#define Robot_h

#include "BufferedInput.h"
#include "SerialPort.h"
#include <sstream>
#include <iostream>
#include <string>
#include <memory>

struct RobotStatus
{
   int echoTime = 0;
};

template <typename RobotConnection>
class GenericRobot
{
   public:
      GenericRobot(const std::string & name, uint32_t baudRate, bool show, bool showDebug)
         : show(show),
           showDebug(showDebug),
           robot(name.c_str(), baudRate),
           input(&robot, 1024)
      {
         assert(&name);
      }

      // Sends a string of bytes to the robot and receives any data the robot responds with.
      // The robot can respond with multiple lines.
      // Each line must end with a '\n' character.
      // The robot must end its response with the line "ok\n".
      void Send(const std::string & str)
      {
         assert(&str);

////         if(show)
////            std::cout << "Robot is executing command." << std::endl;

         if(show)
            std::cout << "To robot: " << str << std::endl;

         robot.Send(str.c_str(), str.length());

         Receive();

////         if(show)
////            std::cout << "Robot has finished." << std::endl;
      }

      void Show(bool show)
      {
         this->show = show;
      }

      void ShowDebug(bool showDebug)
      {
         this->showDebug = showDebug;
      }

      const RobotStatus & GetRobotStatus() const
      {
         return robotStatus;
      }

   private:
      // This function receives lines from the robot until the line "ok" is received.
      void Receive()
      {
         std::string line;

         while(true)
         {
            while((line = input.ReadLine()).empty());

            if(show)
               std::cout << "From robot: " << line << std::endl;

            RobotResponse(line);

            if(EndsWith(line, "ok"))
               return;
         }
      }

      // Handles each line of response from the robot
      // and updates the local copy of the robot's status (robotStatus).
      void RobotResponse(const std::string & response)
      {
         assert(&response);

         std::stringstream responseStream(response);

         std::string responseType;
         responseStream >> responseType;

         if(responseType == "echo")
            responseStream >> robotStatus.echoTime;

         else if(responseType == "debug")
         {
            if(showDebug)
               std::cout << "Robot debug: " << &response[6] << std::endl;
         }

         // This should always be the last of the else-ifs.
         else if(responseType != "ok")
            std::cout << "Warning. Robot sent garbage: " << response << std::endl;
      }

      bool EndsWith(const std::string & str1, const std::string & str2) const
      {
         return str1.find_last_of(str2) == str1.length() - 1;
      }

      bool show;
      bool showDebug;
      RobotConnection robot;
      BufferedInput<RobotConnection *> input;
      RobotStatus robotStatus;
};

#include <queue>

#ifndef UseMockRobot

   using Robot = GenericRobot<SerialPort>;

#else

   #include <Thread.h>

   // The MockRobotSerialPort class is used to test the program without interfacing with a physical robot.
   // It can be turned on by defining UseMockRobot.
   class MockRobotSerialPort
   {
      public:
         MockRobotSerialPort(const char * name, uint32_t baudRate)
         {
            assert(name);
         }

         void Send(const void * data, size_t nBytes)
         {
            assert(data);

            const std::string strData = (char *)data;

            if(strData == "p")
               AddToReads("echo 123456789");

            AddToReads("ok");

            Thread::Sleep(1000);
         }

         size_t Receive(void * data, size_t nBytes)
         {
            assert(data);
            assert(!reads.empty());

            const auto & str = reads.front();
            const auto maxBytes = str.length() < nBytes ? str.length() : nBytes;
            const size_t nBytesToRead = rand() % (maxBytes + 1);

            memcpy(data, str.c_str(), nBytesToRead);

            if(nBytesToRead == str.length())
               reads.pop();
            else
               reads.front() = str.substr(nBytesToRead, str.length() - nBytesToRead);

            return nBytesToRead;
         }

      private:
         void AddToReads(const std::string & line)
         {
            reads.push(line + "\r\n\r");
         }

         std::queue<std::string> reads;
   };

   using Robot = GenericRobot<MockRobotSerialPort>;

#endif

#endif
