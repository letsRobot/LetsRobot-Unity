#include "TcpSocket.h"
#include "Tokenizer.h"
#include "BufferedInput.h"
#include <Thread.h>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <exception>
#include <cassert>
#include <cstdint>

class IrcClientObserver
{
   public:
      virtual ~IrcClientObserver() noexcept { }

      virtual void IrcWelcome() { }
      virtual void IrcMessage(const char * from, const char * to, const char * message) { }
      virtual void IrcNotice(const char * from, const char * to, const char * notice) { }
      virtual void IrcBadMessage(const char * message) { }
};

class IrcBadMessage { };

template <typename Connection>
class GenericIrcClient
{
   public:
      GenericIrcClient(const char * server, uint16_t port, const char * username, const char * password, uint32_t secondsBetweenMessages = 0)
         : connection(server, port),
           input(&connection, 16 * 1024),
           stopped(false),
           secondsBetweenMessages(secondsBetweenMessages)
      {
         assert(server);
         assert(username);
         assert(password);

         UpdateLastMessageTime();
         Init(username, password);
      }

      ~GenericIrcClient() noexcept
      {
         Stop();
      }

      Connection & GetConnection()
      {
         return connection;
      }

      void SetObserver(IrcClientObserver * observer) noexcept
      {
         assert(observer);

         this->observer = observer;
      }

      void JoinChannel(const char * channel)
      {
         assert(channel);

         if(joinedChannel)
            LeaveCurrentChannel();

         std::stringstream message;
         message << "JOIN :" << channel << "\r\n";
         Send(message);

         currentChannel = channel;
         joinedChannel = true;
      }

      void SetSecondsBetweenMessages(uint32_t secondsBetweenMessages)
      {
         this->secondsBetweenMessages = secondsBetweenMessages;
      }

      void SendMessageToChannel(const char * message)
      {
         assert(message);
         assert(joinedChannel);

         SendMessageTo(currentChannel.c_str(), message);
      }

      void SendMessageTo(const char * to, const char * message)
      {
         assert(to);
         assert(message);

         std::stringstream privmsgMessage;
         privmsgMessage << "PRIVMSG " << to << " :" << message <<"\r\n";

         Send(privmsgMessage);
      }

      void NextMessage()
      {
         message = input.ReadLine();
         const auto parsedMessage = ParseMessage(message);

         try
         {
            DispatchMessage(parsedMessage);
         }
         catch(IrcBadMessage &)
         {
            observer->IrcBadMessage(message.c_str());
         }
      }

      const char * GetLastMessage() const noexcept
      {
         return message;
      }

      void Stop()
      {
         stopped = true;
         input.Stop();
      }

   private:
      struct Message
      {
         std::string prefixName;
         std::string command;
         std::string rawParameters;
         std::vector<std::string> parameters;

         size_t GetNumberOfParameters() const noexcept
         {
            return parameters.size();
         }

         const std::string & GetParameter(size_t i) const noexcept
         {
            assert(i < parameters.size());

            return parameters[i];
         }
      };

      void Init(const char * username, const char * password)
      {
         assert(username);
         assert(password);

         this->username = username;
         this->password = password;

         observer = &emptyObserver;
         Login(username, password);
      }

      void Login(const char * username, const char * password)
      {
         assert(username);
         assert(password);

         std::stringstream message;
         message << "PASS :" << password << "\r\n";
         message << "NICK :" << username << "\r\n";

         Send(message);
      }

      void LeaveCurrentChannel()
      {
         std::stringstream partMessage;
         partMessage << "PART :" << currentChannel << "\r\n";

         Send(partMessage);

         joinedChannel = false;
      }

      void Send(const std::stringstream & message)
      {
         assert(&message);

         WaitForMessageCooldown();

         if(message.str().length() > 512)
         {
            std::string cutMessage = message.str().substr(0, 512);
            cutMessage[510] = '\r';
            cutMessage[511] = '\n';
            connection.Send(cutMessage.c_str(), cutMessage.length());
         }
         else
            connection.Send(message.str().c_str(), message.str().length());

         UpdateLastMessageTime();
      }

      Message ParseMessage(const std::string & message)
      {
         assert(&message);

         if(message.length() == 0)
            throw IrcBadMessage();

         Message parsedMessage;
         Tokenizer tokenizer(message.c_str(), message.length());

         ParsePrefix(tokenizer, parsedMessage.prefixName);
         ParseCommand(tokenizer, parsedMessage.command);
         ParseParameters(tokenizer, parsedMessage.parameters, parsedMessage.rawParameters);

         return std::move(parsedMessage);
      }

      void ParsePrefix(Tokenizer & tokenizer, std::string & prefixName)
      {
         assert(&tokenizer);
         assert(&prefixName);

         if(message[0] == ':')
         {
            tokenizer.Skip(1);
            tokenizer.SetDelimiters(" !");
            if(tokenizer.CurrentIsDelimiter())
               throw IrcBadMessage();

            size_t prefixBegin;
            size_t prefixEnd;
            tokenizer.GetNext(prefixBegin, prefixEnd);

            // Skip the rest of the prefix if there is more
            if(tokenizer.GetLastDelimiter() != ' ')
            {
               tokenizer.SetDelimiters(" ");
               tokenizer.SkipNextToken();
            }

            prefixName = std::move(std::string(message.c_str() + prefixBegin, prefixEnd - prefixBegin));
         }
      }

      void ParseCommand(Tokenizer & tokenizer, std::string & command)
      {
         assert(&tokenizer);
         assert(&command);

         tokenizer.SetDelimiters(" ");
         size_t commandBegin;
         size_t commandEnd;
         tokenizer.GetNext(commandBegin, commandEnd);

         if(commandBegin == commandEnd)
            throw IrcBadMessage();

         command = std::move(std::string(message.c_str() + commandBegin, commandEnd - commandBegin));
      }

      void ParseParameters(Tokenizer & tokenizer, std::vector<std::string> & parameters, std::string & rawParameters)
      {
         assert(&tokenizer);
         assert(&parameters);
         assert(&rawParameters);

         tokenizer.SkipDelimiters();
         rawParameters = std::move(std::string(message.c_str() + tokenizer.GetPosition(), message.length() - tokenizer.GetPosition()));

         bool lastParameter = false;
         while(tokenizer.HasMore() && !lastParameter)
         {
            size_t parameterBegin;
            size_t parameterEnd;
            tokenizer.GetNext(parameterBegin, parameterEnd);

            const auto firstChar = message[parameterBegin];

            if(firstChar == ':') // Last parameter?
            {
               lastParameter = true;
               parameterBegin++;
               parameterEnd = tokenizer.GetSize();
            }

            parameters.push_back(std::string(message.c_str() + parameterBegin, parameterEnd - parameterBegin));
         }
      }

      void DispatchMessage(const Message & message)
      {
         assert(&message);

         const auto & command = message.command;

         if(command == "PRIVMSG")
            DispatchPrivmsg(message);

         else if(command == "NOTICE")
            DispatchNotice(message);

         else if(IsNumericCommand(command))
            DispatchNumericCommand(message);

         else if(command == "PING")
            DispatchPing(message);
      }

      bool IsNumericCommand(const std::string & command)
      {
         assert(&command);

         return command.length() == 3 && isdigit(command[0])   &&
                                         isdigit(command[1])   &&
                                         isdigit(command[2]);
      }

      void DispatchPrivmsg(const Message & message)
      {
         assert(&message);

         if(message.prefixName.empty() || message.GetNumberOfParameters() != 2)
            throw IrcBadMessage();

         observer->IrcMessage(message.prefixName.c_str(), message.GetParameter(0).c_str(), message.GetParameter(1).c_str());
      }

      void DispatchNotice(const Message & message)
      {
         assert(&message);

         if(message.prefixName.empty() || message.GetNumberOfParameters() != 2)
            throw IrcBadMessage();

         observer->IrcNotice(message.prefixName.c_str(), message.GetParameter(0).c_str(), message.GetParameter(1).c_str());
      }

      void DispatchNumericCommand(const Message & message)
      {
         assert(&message);

         if(message.GetNumberOfParameters() != 2)
            throw IrcBadMessage();

         if(message.GetParameter(0) != username)
            return;

         switch(strtol(message.command.c_str(), 0, 10))
         {
            case 001:
               observer->IrcWelcome();
               break;
         }
      }

      void DispatchPing(const Message & message)
      {
         assert(&message);

         const std::string pongMessage = "PONG " + message.rawParameters + "\r\n";
         connection.Send(pongMessage.c_str(), pongMessage.length());
      }

      void WaitForMessageCooldown() const
      {
         while(true)
         {
            const auto currentTime = time(0);
            const auto secondsSinceLastMessage = difftime(currentTime, lastMessageTime);

            if(secondsSinceLastMessage >= secondsBetweenMessages)
               break;

            Thread::Sleep(100);
         }
      }

      void UpdateLastMessageTime()
      {
         lastMessageTime = time(0);
      }

      Connection connection;
      std::string username;
      std::string password;
      bool joinedChannel;
      std::string currentChannel;
      IrcClientObserver emptyObserver;
      IrcClientObserver * observer;
      BufferedInput<decltype(&connection), TimedOutTcpSocketException> input;
      std::string message;
      volatile bool stopped;
      uint32_t secondsBetweenMessages;
      time_t lastMessageTime;
};

using IrcClient = GenericIrcClient<TcpClientSocket>;
