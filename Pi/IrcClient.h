#ifndef IrcClient_h
#define IrcClient_h

#include "SimpleTcpConnection.h"
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <cstdint>
#include <cassert>

class IrcClientObserver
{
   public:
      virtual void IrcNumeric(uint32_t numeric) { }
      virtual void IrcMessage(const std::string & from, const std::string & to, const std::string & message) { }
      virtual void IrcNotice(const std::string & notice) { }
};

class IrcClient
{
   public:
      // This is the function called when IRCMessageReceiver is initialized
      IrcClient(const char * server, uint16_t port, const char * nickname, const char * password, IrcClientObserver * observer) : 
         observer(observer),
         connection(server, port), // This is actually a function call that establishes a TCP connection to the server
         bufferSize(256 * 1024),
         buffer(new char[bufferSize]), // Create a 256 KB buffer
         iBuffer(0),
         bufferEnd(0)
      {
         // Make sure all of the parameters are non-zero
         assert(server);
         assert(nickname);
         assert(password);
         assert(*server);
         assert(*nickname);
         assert(*password);
         assert(observer);
         // Log in to IRC server through the TCP connection
         Login(nickname, password);
      }

      void JoinChannel(const char * channelName)
      {
         const auto strJoin = std::string() + "JOIN " + channelName + "\r\n";
         connection.Send(strJoin);
      }

      void ReceiveMessages()
      {
         // Keep receiving messages ... forever?
         // Infinite loop
         while(true)
         {
            Message message;
            // If there is no message to read, try again
            // This is a form of "busy waiting" or "spin locking"
            // It's possible that this is using a lot more battery than it needs to
            // TODO: consider adding some kind of sleep timer here to only check for
            // messages every X milliseconds
            if(!ReadMessage(message))
               continue;

            // Respond to various kinds of message
            if(message.command == "PING")
            {
               const auto response = std::string() + "PONG " + message.parameters + "\r\n";
               connection.Send(response);
            }

            else if(message.command == "PRIVMSG")
            {
               // Separate the message into two parts around the first space character ' '
               size_t usernameEnd = 0;
               const auto from = GetToken(message.parameters, usernameEnd, " ");
               const size_t messageBegin = usernameEnd + 1;
               // Something went wrong, e.g. message had no space in it
               // Quit
               if(messageBegin >= message.parameters.length())
                  return;
               // Send bisected message to IRC observer
               observer->IrcMessage(message.nickname, from, &message.parameters[messageBegin]);
            }

            else if(message.command == "NOTICE")
            {
               // Split message into two parts around : character
               size_t noticeStart = 0;
               GetToken(message.parameters, noticeStart, ":");
               const auto notice = &message.parameters[noticeStart];
               // Send the second part of message to observer (the part after the colon)
               observer->IrcNotice(notice);
            }

            else if(message.command.length() == 3 &&
                    std::isdigit(message.command[0]) &&
                    std::isdigit(message.command[1]) &&
                    std::isdigit(message.command[2]))
            {
               // The message is 3 numeric characters, like "123" or "517"
               // Send this value as an integer to observer
               observer->IrcNumeric(atoi(message.command.c_str()));
            }

            // Any other kinds of messages could be added here
         }
      }

      void SendMessage(const char * to, const char * message)
      {
         const auto sendCommand = std::string("PRIVMSG ") + to + " :" + message + "\r\n";
         connection.Send(sendCommand);
      }

   private:
      struct Message
      {
         std::string original;
         std::string nickname;
         std::string command;
         std::string parameters;
      };

      void Login(const char * nickname, const char * password)
      {
         // Create a login string that the IRC server can parse
         const auto strLogin = std::string() + "PASS " + password + "\r\n" + "NICK " + nickname + "\r\n";
         // Send the login string over the TCP connection
         connection.Send(strLogin);
      }

      std::string ReadLine()
      {
         std::string line;
         while(true)
         {
            const auto c = ReadChar();

            if(c == '\r')
               break;

            if(c == '\n')
               continue;

            line += c;
         }

         return std::move(line);
      }

      char ReadChar()
      {
         if(iBuffer == bufferEnd)
         {
            bufferEnd = connection.Receive(buffer.get(), bufferSize);
            iBuffer = 0;
         }

         return buffer[iBuffer++];

      }

      bool ReadMessage(Message & message)
      {
         const auto messageDelimiters = " \r";
         const auto strMessage = ReadLine();

         if(strMessage.length() == 0)
            return false;

         size_t position = 0;
         std::string prefix;

         const bool messageHasPrefix = strMessage[0] == ':';
         if(messageHasPrefix)
         {
            position = 1; // Skip ':'
            prefix = GetToken(strMessage, position, messageDelimiters);
            if(prefix == "")
               return false;
         }

         const auto command = GetToken(strMessage, position, messageDelimiters);
         if(command == "")
            return false;

         size_t nicknamePosition = 0;
         const auto nickname = GetToken(prefix, nicknamePosition, "!");

         message.original   = strMessage;
         message.nickname   = nickname;
         message.command    = command;
         message.parameters = &strMessage[position];

         return true;
      }

      std::string GetToken(const std::string & str, size_t & position, const std::string & delimiters)
      {
         for(size_t i = position; i < str.length(); i++)
            for(auto delimiter : delimiters)
               if(str[i] == delimiter)
               {
                  const auto oldPosition = position;
                  const auto length = i - position;

                  position = i + 1; // Skip spaces
                  while(position < str.length() && str[position] == ' ')
                     position++;

                  return std::move(str.substr(oldPosition, length));
               }

         return "";
      }

      IrcClientObserver * const observer;
      SimpleTcpConnection connection;
      const size_t bufferSize;
      std::unique_ptr<char[]> buffer;
      size_t iBuffer;
      size_t bufferEnd;
};

#endif
