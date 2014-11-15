// To add a new command add the following
//
//    Command("<command description>")
//    {
//        <code to handle command>
//    }
//
// where <command description> is the first of the command descriptions (the one before the alternative command descriptions) for the given command in Commands.txt.
//
//
// Each command function has the following objects available:
// - parameters
// - robot
// - irc
// - unity
// - messageObserver
//
// parameters
//
// The parameters object contains the actual parameters in the invocation of the command.
// It has type ActualCommandParameters.
// To get the value of one of the parameters the functions GetWord, GetString, and GetInteger can be used with the (zero-based) index of the wanted parameter as parameter.
// The type of the value you're trying to get must match the type of the formal parameter.
// GetWord must match #w, GetString must match #s, and GetInteger must match #i.
// For example, if for the command "do stuff #w #i #w #s" the following calls are valid:
// parameters.GetWord(0), parameters.GetInteger(1), parameters.GetWord(2), parameters.GetString(3).
//
//
// robot
//
// The robot object can be used to send strings to the robot and to get the current status of the robot.
// Currently the only status available is the echo distance.
// It has type Robot.
// The Send function sends a string to the robot excluding the terminating null character.
// The status of the robot can be retrieved using the GetRobotStatus function.
//
//
// irc
//
// The irc object can be used to send messages or actions to the IRC server.
// It has type IrcThread.
// Messages, whispers, and actions (/me) can be sent using the functions SendMessage, SendWhisper, and SendActionMessage respectively.
//
//
// unity
//
// The unity object can be used to send chat messages and variables to the Unity program.
// It has type Unity.
// Chat messages can be sent using the function SendChatMessage while variables can be sent using the function SendVariableMessage.
//
//
// messageObserver
//
// The messageObserver object can be used to add a chat message to the chat message queue as if it had come from the actual chat.
// It has type MessageObserver.
// Chat messages can be added using the function NewMessage.


#include "Commands.h"
#include "Lights.h"
#include <map>

Command("/show #w")
{
   ExecuteShowHide(CommandFunctionActualParameters, true);
}

Command("/hide #w")
{
   ExecuteShowHide(CommandFunctionActualParameters, false);
}

Command("/say #s")
{
   irc.SendMessage(parameters.GetString(0).c_str());

   messageObserver.NewMessage(true, irc.GetUsername(), parameters.GetString(0));
}

Command("/me #s")
{
   irc.SendActionMessage(parameters.GetString(0).c_str());

   std::string message = "\001ACTION";
   message += parameters.GetString(0);
   message += "\001";
   messageObserver.NewMessage(true, irc.GetUsername(), message);
}

Command("forward")
{
   robot.Send("f");
}

Command("back")
{
   robot.Send("b");
}

Command("left")
{
   robot.Send("l");
}

Command("right")
{
   robot.Send("r");
}

Command("poke")
{
   robot.Send("k");
}

Command("open")
{
   robot.Send("o");
}

Command("close")
{
   robot.Send("g");
}

Command("echo")
{
   robot.Send("p");

   const int distance = (0.03448 * robot.GetRobotStatus().echoTime) / 2 + 0.5;
   std::stringstream str;
   str << "Distance to object: " << distance << "cm.";

   irc.SendMessage(str.str().c_str());
   messageObserver.NewMessage(true, irc.GetUsername(), str.str().c_str());
   unity.SendVariableMessage("echo", std::to_string(distance));
}

Lights lights("/dev/i2c-1", 0x04);
const auto numberOfLights = 16;
const uint8_t maxIntensity = 100;

Command("light #i #w")
{
   const auto light = parameters.GetInteger(0);
   const auto color = parameters.GetWord(1);

   if(light >= numberOfLights)
      return;

   Light(CommandFunctionActualParameters, light, color);
}

Command("light all #w")
{
   const auto color = parameters.GetWord(0);

   Light(CommandFunctionActualParameters, 255, color);
}

Command("light #i #i #i #i")
{
   const auto light = parameters.GetInteger(0);
   const auto r     = parameters.GetInteger(1);
   const auto g     = parameters.GetInteger(2);
   const auto b     = parameters.GetInteger(3);

   if(light >= numberOfLights)
      return;

   Light(CommandFunctionActualParameters, light, r, g, b);
}

Command("light all #i #i #i")
{
   const auto r = parameters.GetInteger(0);
   const auto g = parameters.GetInteger(1);
   const auto b = parameters.GetInteger(2);

   Light(CommandFunctionActualParameters, 255, r, g, b);
}

void ExecuteShowHide(CommandFunctionParameters, bool showHide)
{
   assert(&parameters);

   std::string parameter = parameters.GetWord(0);
   const bool all = parameter == "all";

   if(all || parameter == "chat")
      showChat = showHide;

   if(all || parameter == "commands")
      showCommands = showHide;

   if(all || parameter == "robot")
      robot.Show(showHide);

   if(all || parameter == "robot_debug")
      robot.ShowDebug(showHide);
}

void Light(CommandFunctionParameters, int light, const std::string & color)
{
   struct Rgb
   {
      uint8_t r, g, b;
   };

   const std::map<std::string, Rgb> colors =
   {
      {"black",   {0,   0,   0  }},
      {"red",     {255, 0,   0  }},
      {"green",   {0,   255, 0  }},
      {"blue",    {0,   0,   255}},
      {"yellow",  {255, 255,   0}},
      {"purple",  {255, 0,   255}},
      {"magenta", {255, 0,   255}},
      {"cyan",    {0,   255, 255}},
      {"white",   {255, 255, 255}}
   };

   const auto iColor = colors.find(color);

   if(iColor == colors.end())
      return;

   auto rgb = iColor->second;

   rgb.r = rgb.r * (double)maxIntensity / 255 + 0.5;
   rgb.g = rgb.g * (double)maxIntensity / 255 + 0.5;
   rgb.b = rgb.b * (double)maxIntensity / 255 + 0.5;

   SetLight(CommandFunctionActualParameters, light, rgb.r, rgb.g, rgb.b);
}

void Light(CommandFunctionParameters, int light, int r, int g, int b)
{
   SetLight(CommandFunctionActualParameters, light, r, g, b);
}

void SetLight(CommandFunctionParameters, int light, int r, int g, int b)
{
   lights.SetLight(light, r, g, b);

   int iLightFirst;
   int iLightEnd;

   if(light == 255)
   {
      iLightFirst = 0;
      iLightEnd = numberOfLights;
   }
   else
   {
      iLightFirst = light;
      iLightEnd   = light + 1;
   }

   for(int iLight = iLightFirst; iLight != iLightEnd; iLight++)
   {
      std::string variable;
      variable += "led_";
      variable += std::to_string(iLight);

      std::string value;
      value += std::to_string(r);
      value += " ";
      value += std::to_string(g);
      value += " ";
      value += std::to_string(b);

      unity.SendVariableMessage(variable, value);
   }
}
