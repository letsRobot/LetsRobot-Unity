#ifndef CommandDescriptions_h
#define CommandDescriptions_h

#include "ActualCommandParameters.h"
#include "Tokenizer.h"
#include "ExceptionWithoutDescription.h"
#include <fstream>
#include <string>
#include <utility>
#include <cstdint>

enum class CommandPartType
{
   Keyword,
   Word,
   Integer,
   String
};

struct CommandDescriptionPart
{
   CommandPartType type;
   std::string keyword;

   CommandDescriptionPart(CommandPartType type, const std::string & keyword)
      : type(type),
        keyword(keyword)
   {
      assert(&keyword);
   }
};

// The class CommandDescription represents a description of a command and its parameters.
// Since one command can be invoked in multiple ways (e.g. 'left', 'turn left', and 'go left' could invoke the same command) a CommandDescription object contains a list of descriptions of ways to invoke the command.
// The description of each way to invoke the command consists of a number of CommandDescriptionPart objects, each representing either a keyword, a string, or an integer.
// For example 'led #i #s' and 'light #i #s' gives two ways to invoke the same command. The description of the first way consists of three parts: the keyword 'led', an integer, and a string.
class CommandDescription
{
   public:
      void SetPrivilegeLevel(int32_t privilegeLevel)
      {
         this->privilegeLevel = privilegeLevel;
      }

      void SetCooldownTime(uint32_t cooldownTime)
      {
         this->cooldownTime = cooldownTime;
      }

      void AddPart(CommandPartType type, const std::string & keyword = "")
      {
         assert(&keyword);

         CommandDescriptionPart part(type, keyword);
         currentCommandDescription.push_back(part);
      }

      void CommitCommandDescription()
      {
         commandDescriptions.push_back(currentCommandDescription);
         currentCommandDescription.clear();
      }

      int32_t GetPrivilegeLevel() const
      {
         return privilegeLevel;
      }

      uint32_t GetCooldownTime() const
      {
         return cooldownTime;
      }

      const std::vector<std::vector<CommandDescriptionPart>> & GetCommandDescriptions() const
      {
         return commandDescriptions;
      }

      std::string GetString(size_t iSingleCommandDescription = 0) const
      {
         assert(iSingleCommandDescription < commandDescriptions.size());

         std::string str;

         for(const auto & part : commandDescriptions[iSingleCommandDescription])
         {
            if(!str.empty())
               str += ' ';

            switch(part.type)
            {
               case CommandPartType::Keyword:
                  str += part.keyword;
                  break;

               case CommandPartType::Word:
                  str += "#w";
                  break;

               case CommandPartType::Integer:
                  str += "#i";
                  break;

               case CommandPartType::String:
                  str += "#s";
                  break;

               default:
                  assert(false);
            }
         }

         return std::move(str);
      }

   private:
      std::vector<std::vector<CommandDescriptionPart>> commandDescriptions;
      int32_t privilegeLevel;
      uint32_t cooldownTime;
      std::vector<CommandDescriptionPart> currentCommandDescription;
};

struct ActualCommandPart
{
   CommandPartType type;
   std::string str;
   int32_t i;

   ActualCommandPart(const std::string & str)
      : type(CommandPartType::Word),
        str(str),
        i(0)
   {
      assert(&str);
   }

   ActualCommandPart(int32_t i)
      : type(CommandPartType::Integer),
        i(i)
   { }
};

// The class ActualCommand represents an invocation command with associated parameter values as opposed to merely a description of a command.
// For instance the command 'light 10 blue' is an actual command consisting of the keyword 'left', the integer 10, and the string 'blue'.
class ActualCommand
   : public ActualCommandParameters
{
   public:
      // Parses a string into a list of ActualCommandPart objects.
      ActualCommand(const std::string & command)
         : strCommand(command),
           commandDescription(0),
           iSingleCommandDescription(0)
      {
         assert(&command);

         Tokenizer tokenizer(command.c_str(), command.length(), " ");

         while(tokenizer.HasMore())
         {
            const auto tokenPosition = tokenizer.GetPosition();
            const auto token = tokenizer.GetNext();

            char * strtolEnd = 0;
            int32_t i = strtol(token.c_str(), &strtolEnd, 10);
            const bool tokenIsInteger = strtolEnd == &token[0] + token.length();

            if(tokenIsInteger)
               AddPart(ActualCommandPart(i), tokenPosition);
            else
               AddPart(ActualCommandPart(token), tokenPosition);
         }
      }

      void SetCommandDescription(const CommandDescription * commandDescription, size_t iSingleCommandDescription)
      {
         assert(commandDescription);

         this->commandDescription = commandDescription;
         this->iSingleCommandDescription = iSingleCommandDescription;
      }

      // May return a null pointer.
      const CommandDescription * GetCommandDescription() const
      {
         return commandDescription;
      }

      size_t GetNumberOfParts() const
      {
         return parts.size();
      }

      const ActualCommandPart & GetPart(size_t iPart) const
      {
         assert(iPart < parts.size());

         return parts[iPart];
      }

      const char * GetWord(size_t iParameter) const
      {
         size_t iPart;
         if(GetIndexOfPartOfParameter(iParameter, CommandPartType::Word, iPart))
            return parts[iPart].str.c_str();

         return "";
      }

      const char * GetString(size_t iParameter) const
      {
         size_t iPart;
         if(GetIndexOfPartOfParameter(iParameter, CommandPartType::String, iPart))
            return &strCommand[partPositionsInMessage[iPart]];

         return "";
      }

      int32_t GetInteger(size_t iParameter) const
      {
         size_t iPart;
         if(GetIndexOfPartOfParameter(iParameter, CommandPartType::Integer, iPart))
            return parts[iPart].i;

         return 0;
      }

   private:
      void AddPart(const ActualCommandPart & part, size_t partPositionInMessage)
      {
         assert(&part);

         parts.push_back(part);
         partPositionsInMessage.push_back(partPositionInMessage);
      }

      bool GetIndexOfPartOfParameter(size_t iParameter, CommandPartType type, size_t & iPart) const
      {
         assert(commandDescription);
         assert(type != CommandPartType::Keyword);

         const auto & parts = commandDescription->GetCommandDescriptions()[iSingleCommandDescription];

         iPart = 0;
         size_t iCurrentParameter = 0;

         for(const auto & part : parts)
         {
            if(part.type != CommandPartType::Keyword)
            {
               if(iCurrentParameter == iParameter && part.type == type)
                  return true;

               iCurrentParameter++;
            }

            iPart++;
         }

         const auto nParameters = iCurrentParameter;
         const auto & strCommandDescription = commandDescription->GetString(iSingleCommandDescription);

         if(iParameter >= nParameters)
            std::cout << "Internal error. Parameter index " << iParameter << " is out of range for command '" << strCommandDescription << "'." << std::endl;
         else
         {
            std::cout << "Internal error. Bad parameter type (parameter index " << iParameter << ") in command '" << strCommandDescription << "'." << std::endl;
         }



         return false;
      }

      const std::string & strCommand;
      const CommandDescription * commandDescription;
      size_t iSingleCommandDescription;
      std::vector<ActualCommandPart> parts;
      std::vector<size_t> partPositionsInMessage;
};

// The class CommandDescriptions loads command descriptions from a file and can be used to tell whether an actual command is valid or not.
class CommandDescriptions
{
   public:
      CommandDescriptions(const char * filename)
      {
         assert(filename);

         ParseFile(filename);
      }

   void FindCorrespondingCommandDescription(ActualCommand & actualCommand) const
   {
      assert(&actualCommand);

      // For each command description
      for(const auto & commandDescription : commandDescriptions)
      {
         // For each way of invoking that command
         size_t iSingleCommandDescription = 0;
         for(const auto & singleCommandDescription : commandDescription.GetCommandDescriptions())
         {
            if(CommandsCorresponds(actualCommand, singleCommandDescription))
            {
               actualCommand.SetCommandDescription(&commandDescription, iSingleCommandDescription); // We found one.
               return;
            }

            iSingleCommandDescription++;
         }
      }
   }

   private:
      void ParseFile(const char * filename)
      {
         assert(filename);

         std::ifstream file(filename, std::ios_base::binary);
         if(!file.is_open())
         {
            std::cout << "Failed to open " << filename << "." << std::endl;
            throw 0;
         }

         currentLine = 0;

         std::string line;
         while(true)
         {
            getline(file, line);
            if(file.eof())
               break;

            if(!file.good())
            {
               std::cout << "Failed to read " << filename << "." << std::endl;
               throw ExceptionWithoutDescription();
            }

            currentLine++;
            ParseCommandDescriptionLine(line, filename);
         }
      }

      void ParseCommandDescriptionLine(const std::string & line, const char * filename)
      {
         assert(&line);
         assert(filename);

         const auto tokens = TokenizeString(line);

         if(tokens.size() == 0) // Current line is empty
         {
            if(!currentCommandDescription.GetCommandDescriptions().empty())
               commandDescriptions.push_back(currentCommandDescription);

            currentCommandDescription = CommandDescription();
            state = State::PrivilegeLevel;
            return;
         }

         else if(tokens[0][0] == '#') // Current line is a comment
            return;

         else if(state == State::PrivilegeLevel)
         {
            char * lastChar;
            const auto privilegeLevel = strtol(tokens[0].c_str(), &lastChar, 10);
            if(*lastChar)
            {
               std::cout << "Bad privilege level in " << filename << " on line " << currentLine << "." << std::endl;
               throw ExceptionWithoutDescription();
            }

            currentCommandDescription.SetPrivilegeLevel(privilegeLevel);
            state = State::Cooldown;
         }

         else if(state == State::Cooldown)
         {
            char * lastChar;
            const auto cooldownTime = strtol(tokens[0].c_str(), &lastChar, 10);
            if(*lastChar)
            {
               std::cout << "Bad cooldown time in " << filename << " on line " << currentLine << "." << std::endl;
               throw ExceptionWithoutDescription();
            }

            currentCommandDescription.SetCooldownTime(cooldownTime);
            state = State::CommandDescription;
         }

         else if(state == State::CommandDescription)
         {
            for(const auto & commandPart : tokens)
            {
               CommandPartType type;
               std::string keyword;

               if(commandPart == "#i")
                  type = CommandPartType::Integer;

               else if(commandPart == "#w")
                  type = CommandPartType::Word;

               else if(commandPart == "#s")
                  type = CommandPartType::String;

               else
               {
                  type = CommandPartType::Keyword;
                  keyword = commandPart;
               }

               currentCommandDescription.AddPart(type, keyword);
            }

            currentCommandDescription.CommitCommandDescription();
         }
      }

      std::vector<std::string> TokenizeString(const std::string & str) const
      {
         assert(&str);

         Tokenizer tokenizer(str.c_str(), str.length(), " \t");

         std::vector<std::string> tokens;

         while(tokenizer.HasMore())
            tokens.push_back(tokenizer.GetNext());

         return std::move(tokens);
      }

      bool CommandsCorresponds(const ActualCommand & actualCommand, const std::vector<CommandDescriptionPart> & singleCommandDescription) const
      {
         assert(&actualCommand);
         assert(&singleCommandDescription);

         if(actualCommand.GetNumberOfParts() < singleCommandDescription.size())
            return false;

         for(size_t iPart = 0; iPart < actualCommand.GetNumberOfParts(); iPart++)
         {
            const auto & actualPart = actualCommand.GetPart(iPart);
            const auto & descriptionPart = singleCommandDescription[iPart];

            if(descriptionPart.type == CommandPartType::String)
               return true;

            if(!PartsAreCompatible(actualPart, descriptionPart))
               return false;
         }

         return true;
      }

      bool PartsAreCompatible(const ActualCommandPart & actualPart, const CommandDescriptionPart & descriptionPart) const
      {
         assert(&descriptionPart);

         return PartsAreKeywords(actualPart, descriptionPart) ||
                PartsAreWords   (actualPart, descriptionPart) ||
                PartsAreIntegers(actualPart, descriptionPart);
      }

      bool PartsAreKeywords(const ActualCommandPart & actualPart, const CommandDescriptionPart & descriptionPart) const
      {
         assert(&descriptionPart);

         return descriptionPart.type == CommandPartType::Keyword &&
                actualPart.type      == CommandPartType::Word    &&
                actualPart.str       == descriptionPart.keyword;
      }

      bool PartsAreWords(const ActualCommandPart & actualPart, const CommandDescriptionPart & descriptionPart) const
      {
         assert(&descriptionPart);

         return actualPart.type      == CommandPartType::Word &&
                descriptionPart.type == CommandPartType::Word;
      }

      bool PartsAreIntegers(const ActualCommandPart & actualPart, const CommandDescriptionPart & descriptionPart) const
      {
         assert(&descriptionPart);

         return actualPart.type      == CommandPartType::Integer &&
                descriptionPart.type == CommandPartType::Integer;
      }

      enum class State {PrivilegeLevel, Cooldown, CommandDescription} state = State::PrivilegeLevel;
      int32_t currentLine;
      CommandDescription currentCommandDescription;
      std::vector<CommandDescription> commandDescriptions;
};

#endif
