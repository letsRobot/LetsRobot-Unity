#ifndef RobotSettings_h
#define RobotSettings_h

#include "ExceptionWithoutDescription.h"
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cassert>
#include <cstdlib>
#include <cstdint>

class RobotSettings
{
   public:
      RobotSettings(const char * filename)
      {
         assert(filename);

         std::ifstream file(filename);
         if(!file.is_open())
            Fail(filename);

         while(!file.eof())
         {
            std::string line;
            getline(file, line);

            if(line.empty() || line[0] == '#')
               continue;

            std::stringstream lineStream(line);
            std::string setting;
            std::string equalsSign;
            std::string value;

            lineStream >> setting;
            lineStream >> equalsSign;
            lineStream >> value;

            if(setting.empty())
               continue;

            if((!file.good() && !file.eof()) || equalsSign != "=" || value.empty())
               Fail(filename);

            settings[setting] = value;
         }
      }

      const char * GetString(const char * setting) const
      {
         assert(setting);

         const auto iSetting = settings.find(setting);

         if(iSetting == settings.end())
         {
            std::cout << "Warning: Setting '" << setting << "' was not found." << std::endl;
            return "";
         }

         return iSetting->second.c_str();
      }

      int32_t GetInteger(const char * setting) const
      {
         assert(setting);

         const auto strValue = GetString(setting);

         if(strcmp(strValue, "") == 0)
            return 0;

         char * strtolEnd = 0;
         const auto value = strtol(strValue, &strtolEnd, 10);

         if(*strtolEnd)
         {
            std::cout << "Warning: Setting '" << setting << "' is not an integer." << std::endl;
            return 0;
         }

         return value;
      }

      bool GetBoolean(const char * setting) const
      {
         assert(setting);

         const auto strValue = GetString(setting);

         if(strcmp(strValue, "") == 0)
            return false;

         if(strcmp(strValue, "true") == 0)
            return true;

         if(strcmp(strValue, "false") == 0)
            return false;

         std::cout << "Warning: Setting '" << setting << "' is not a boolean." << std::endl;
         return false;
      }

   private:
      void Fail(const char * filename)
      {
         assert(filename);

         std::cout << "Failed to read settings file (" << filename << ")." << std::endl;
         throw ExceptionWithoutDescription();
      }

      std::map<std::string, std::string> settings;
};

#endif
