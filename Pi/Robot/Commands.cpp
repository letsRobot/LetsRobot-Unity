// To add a new command add the following
//
//      Command("<command description>")
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
//
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
// To send an ordinary messages, whispers, and actions (/me) can be sent using the functions SendMessage, SendWhisper, and SendActionMessage respectively.

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
   irc.SendMessage(parameters.GetString(0));
}

Command("/me #s")
{
   irc.SendActionMessage(parameters.GetString(0));
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
}

Lights lights("/dev/i2c-1", 0x04);

Command("light #i #w")
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
      {"white",   {255, 255, 255}},
   };

   const auto light = parameters.GetInteger(0);
   const std::string color = parameters.GetWord(1);
   const uint8_t maxIntensity = 100;

   const auto iColor = colors.find(color);

   if(iColor == colors.end())
      return;

   auto rgb = iColor->second;

   rgb.r = rgb.r * (double)maxIntensity / 255 + 0.5;
   rgb.g = rgb.g * (double)maxIntensity / 255 + 0.5;
   rgb.b = rgb.b * (double)maxIntensity / 255 + 0.5;

   lights.SetLight(light, rgb.r, rgb.g, rgb.b);

//   std::cout << "LED " << light << " is now " << color << "." << std::endl;
//   std::cout << "LED " << light << " now has RGB color " << (int)rgb.r << " " << (int)rgb.g << " " << (int)rgb.b <<  "." << std::endl;
}

Command("light #i #i #i #i")
{
   const auto light = parameters.GetInteger(0);
   const auto r = parameters.GetInteger(1);
   const auto g = parameters.GetInteger(2);
   const auto b = parameters.GetInteger(3);

   lights.SetLight(light, r, g, b);

//   std::cout << "LED " << parameters.GetInteger(0) << " now has RGB color " << parameters.GetInteger(1) << " " << parameters.GetInteger(2) << " " << parameters.GetInteger(3) <<  "." << std::endl;
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
