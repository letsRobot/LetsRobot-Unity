#include "IrcClient.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>

class IrcMessageReceiver : private IrcClientObserver
{
   public:
      IrcMessageReceiver(const char * password) : robotSerialPort(-1),
                                                  client("irc.twitch.tv", 6667, "aylobot", password, this),
                                                  channel("#aylojill")
      {
         ConnectToRobot("/dev/ttyUSB0");

         client.JoinChannel(channel);
         client.ReceiveMessages();
      }

   private:
      void IrcNumeric(uint32_t numeric)
      {
         if(numeric == 1)
            std::cout << "Connected to Twitch." << std::endl;
      }

      void IrcNotice(const std::string & notice)
      {
         if(notice == "Login unsuccessful") // This is what Twitch says if the login fails. It may change in the future.
            throw "Failed to login.";
      };


      // This function is called every time a user sends a message.
      //
      // The parameter 'from' is the username of the person who sent the message.
      //
      // The parameter 'to' is the username or channel the message was sent to.
      // If 'to' starts with a hashtag as in "#aylojill" the message is sent to everybody.
      // Otherwise it is a private message. (I'm not sure Twitch actually allows for private messages like this.)
      //
      // The parameter 'message' is (surprise) the message.
      void IrcMessage(const std::string & from, const std::string & to, const std::string & message)
      {
         if(from == "jtv") // Ignore messages from the Twitch server
            return;

         if(message == "forward")
            SendToRobot("f");

         else if(message == "back")
            SendToRobot("b");

         else if(message == "left")
            SendToRobot("l");

         else if(message == "right")
            SendToRobot("r");

         else if(message == "echo")
         {
            SendToRobot("p");

            std::stringstream robotResponse(ReceiveFromRobot());
            std::string echo;
            int timeInMs;
            robotResponse >> echo;
            robotResponse >> timeInMs;

            if(echo != "echo")
               throw "Robot should have responded with \"echo\".";

            const int distance = (0.03448 * timeInMs) / 2 + 0.5;

            std::stringstream message;
            message << "Distance is " << distance << " cm.";
            client.SendMessage(channel, message.str().c_str());
         }

         std::cout << "Message from: " << from << " to " << to << ": " << message << std::endl;
      }

      void ConnectToRobot(const char * port)
      {
         robotSerialPort = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
         if(robotSerialPort == -1)
            throw "Failed to connect to robot";

         fcntl(robotSerialPort, F_SETFL, 0);

         std::cout << "Connected to robot." << std::endl;
      }

      void SendToRobot(const std::string & str)
      {
         const auto nBytesWritten = write(robotSerialPort, str.c_str(), str.length());
         if(nBytesWritten < 0)
            throw "Failed to send to robot.";
      }

      std::string ReceiveFromRobot()
      {
         const auto bufferSize = 512;
         char buffer[bufferSize];
         const auto nBytesRead = read(robotSerialPort, buffer, bufferSize);

         return std::string(bufferSize, nBytesRead);
      }

      int robotSerialPort;
      IrcClient client;
      const char * channel;
};

int main(int argc, char ** argv)
{
   if(argc < 2)
   {
      std::cout << "Usage: <program name> <password>" << std::endl;
      return 1;
   }

   try
   {
      IrcMessageReceiver receiver(argv[1]);
   }
   catch(const char * errorMessage)
   {
      std::cout << errorMessage << std::endl;
      return 1;
   }
   catch(std::bad_alloc &)
   {
      std::cout << "Out of memory." << std::endl;
      return 1;
   }
   catch(...)
   {
      std::cout << "Unknown error." << std::endl;
      return 1;
   }
}
