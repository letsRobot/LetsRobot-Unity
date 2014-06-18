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
      IrcClient(const char * server, uint16_t port, const char * nickname, const char * password, IrcClientObserver * observer) : observer(observer),
                                                                                                                                  connection(server, port),
                                                                                                                                  bufferSize(256 * 1024),
                                                                                                                                  buffer(new char[bufferSize]),
                                                                                                                                  bufferBegin(0),
                                                                                                                                  bufferEnd(0)
      {
         assert(server);
         assert(nickname);
         assert(password);
         assert(*server);
         assert(*nickname);
         assert(*password);
         assert(observer);

         Login(nickname, password);
      }

      void JoinChannel(const char * channelName)
      {
         const auto strJoin = std::string() + "JOIN " + channelName + "\r\n";
         connection.Send(strJoin);
      }

      void ReceiveMessages()
      {
         while(true)
         {
            Message message;

            if(!ReadMessage(message))
               continue;

            if(message.command == "PING")
            {
               const auto response = std::string() + "PONG " + message.parameters + "\r\n";
               connection.Send(response);
            }

            else if(message.command == "PRIVMSG")
            {
               size_t usernameEnd = 0;
               const auto from = GetToken(message.parameters, usernameEnd, " ");
               const size_t messageBegin = usernameEnd + 1;

               if(messageBegin >= message.parameters.length())
                  return;

               observer->IrcMessage(message.nickname, from, &message.parameters[messageBegin]);
            }

            else if(message.command == "NOTICE")
            {
               size_t noticeStart = 0;
               GetToken(message.parameters, noticeStart, ":");
               const auto notice = &message.parameters[noticeStart];
               observer->IrcNotice(notice);
            }

            else if(message.command.length() == 3 &&
                    std::isdigit(message.command[0]) &&
                    std::isdigit(message.command[1]) &&
                    std::isdigit(message.command[2]))
            {
               observer->IrcNumeric(atoi(message.command.c_str()));
            }
         }
      }

      void SendMessage(const char * to, const char * message)
      {
         const auto sendCommand = std::string("PRIVMSG ") + to + " " + message + "\r\n";
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
         const auto strLogin = std::string() + "PASS " + password + "\r\n" + "NICK " + nickname + "\r\n";
         connection.Send(strLogin);
      }

      std::string ReadLine()
      {
         FillInputBuffer();

         std::string line;
         while(bufferBegin != bufferEnd)
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
         const auto c = buffer[bufferBegin];
         bufferBegin++;
         FillInputBuffer();

         return c;
      }

      void FillInputBuffer()
      {
         if(bufferBegin != bufferEnd)
            return;

         bufferEnd = connection.Receive(buffer.get(), bufferSize);
         bufferBegin = 0;
         buffer.get()[bufferEnd] = 0;
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
      size_t bufferBegin;
      size_t bufferEnd;
};

#endif
