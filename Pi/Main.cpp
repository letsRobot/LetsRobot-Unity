#include "IrcClient.h"
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

class IrcMessageReceiver : private IrcClientObserver
{
   public:
      IrcMessageReceiver(const char * password) : robotSerialPort(-1)
      {
         ConnectToRobot("/dev/ttyUSB0");

         const auto username = "aylobot";
         const auto channel  = "#aylojill";

         IrcClient client("irc.twitch.tv", 6667, username, password, this);
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
         if(notice == "Login unsuccessful") // This is what Twitch says when if login fails. It may change in the future.
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

      int robotSerialPort;
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
