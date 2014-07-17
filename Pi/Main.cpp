#include "IrcClient.h"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <sstream>
#include <ctime>

class IrcMessageReceiver : private IrcClientObserver
{
   public:
      IrcMessageReceiver(const char * password) : robotSerialPort(-1),
                                                  client("irc.twitch.tv", 6667, "aylobot", password, this),
                                                  channel("#aylojill"),
                                                  timeLastRobotMessage(0)
      {
         ConnectToRobot("/dev/ttyUSB0");

         client.JoinChannel(channel);
         RobotMessage("Systems online. Preparing weapons for human slaughter. I mean... hi, how are ya?");
         client.ReceiveMessages();
      }

      ~IrcMessageReceiver()
      {
         const auto resultClose = close(robotSerialPort);
         assert(resultClose != -1);
      }

   private:
      void IrcNumeric(uint32_t numeric)
      {
         if(numeric == 1)
         {
            RobotMessage("Hi, I'm a robot. No, really.");
            std::cout << "Connected to Twitch." << std::endl;
         }
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

         std::cout << "Message from: " << from << " to " << to << ": " << message << std::endl;

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
            int microseconds;
            robotResponse >> echo;
            robotResponse >> microseconds;

            if(echo != "echo")
            {
               std::cout << "Robot did not respond with \"echo <int>\"." << std::endl;
               return;
            }

            std::stringstream message;
            message << "Echo time: " << microseconds << " microseconds.";
            RobotMessage(message.str().c_str());
         }
      }

      void ConnectToRobot(const char * port)
      {
         if((robotSerialPort = open(port, O_RDWR | O_NOCTTY)) == -1)
            throw "Failed to connect to robot";

         termios portAttributes;
         if(tcgetattr(robotSerialPort, &portAttributes) == -1)
            throw "Failed to get serial port attributes.";

         if(cfsetispeed(&portAttributes, B9600) == -1 ||
            cfsetospeed(&portAttributes, B9600) == -1)
            throw "Failed to set serial port baud rate.";

         cfmakeraw(&portAttributes);
         portAttributes.c_cflag &= ~CSTOPB;
//         portAttributes.c_cflag &= ~CRTSCTS;
         portAttributes.c_cflag |= CLOCAL | CREAD;

         portAttributes.c_cc[VTIME] = 1;
         portAttributes.c_cc[VMIN] = 1;

         if(tcsetattr(robotSerialPort, TCSANOW, &portAttributes) == -1)
            throw "Failed to set serial port attributes.";

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
         std::string input;

         while(true)
         {
            char c;
            const auto nBytesRead = read(robotSerialPort, &c, 1);

            if(nBytesRead <= 0)
               continue;

            if(c == '\n')
               break;

            input += c;
         }

         if(input.back() == '\r')
            return input.substr(0, input.length() - 1);

         return input;
      }

      void RobotMessage(const char * message)
      {
         const auto currentTime = time(0);
         const auto secondsSinceLastMessage = difftime(currentTime, timeLastRobotMessage);
         const auto firstMessage = timeLastRobotMessage == 0;

         if(secondsSinceLastMessage < 2 && !firstMessage)
         {
            std::cout << "Too many message per second. Message dropped: \"" << message << "\"" << std::endl;
            return;
         }

         client.SendMessage(channel, message);
         timeLastRobotMessage = time(0);
      }

      int robotSerialPort;
      IrcClient client;
      const char * channel;
      time_t timeLastRobotMessage;
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
