#ifndef Users_h
#define Users_h

#include "ExceptionWithoutDescription.h"
#include <fstream>
#include <string>
#include <map>
#include <cstdint>
#include <cassert>

class Users
{
   public:
      Users(const char * filename)
      {
         assert(filename);

         std::ifstream file(filename);
         if(!file.is_open())
         {
            std::cout << "Failed to open " << filename << "." << std::endl;
            throw 0;
         }

         uint32_t currentLine = 0;

         while(!file.eof())
         {
            std::string line;
            getline(file, line);

            currentLine++;

            if(line.empty() || line[0] == '#')
               continue;

            std::stringstream lineStream(line);
            std::string user;
            std::string strPrivilegeLevel;

            lineStream >> user;
            lineStream >> strPrivilegeLevel;

            char * lastChar;
            const auto privilegeLevel = strtol(strPrivilegeLevel.c_str(), &lastChar, 10);
            if(*lastChar)
            {
               std::cout << "Bad privilege level in " << filename << " on line " << currentLine << "." << std::endl;
               throw ExceptionWithoutDescription();
            }

            if(user.empty())
               continue;

            users[user] = privilegeLevel;
         }
      }

      int32_t GetPrivilegeLevel(const std::string & user) const
      {
         assert(&user);

         const auto iUser = users.find(user);

         if(iUser == users.end())
            return 0;

         return iUser->second;
      }

   private:
      std::map<std::string, int32_t> users;
};

#endif
