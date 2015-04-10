using UnityEngine;
using System.Collections.Generic;

// The Update and Command functions uses the following parameters:
//
// command
// The command.Is() function can be used to determine what the command is.
// The functions command.GetInteger(), command.GetWord(), and command.GetString() can be used to get the actual parameters of the command.
// They work the same way as the functions of 'parameters' in Commands.cpp do.
//
// robot
// The robot.SendMessage() function can be used to send a message to the Pi program.
// The Pi program will receive this message as if it had typed directly into it.
//
// variable
// The variables parameter contains a number of names each associated with a value.
// These name/value pairs can be set from the Pi as described in Commands.cpp.
// Both the names and the values are of type string.
// To get the value associated with a certain name use variables[name]. E.g. string echoDistance = variables["echo"];
// Trying to get a value using a name that does not have an associated value yet will throw a KeyNotFoundException.

public class RobotStuff
{
	// This function is called before every frame is shown.
	public void Update(IDictionary<string, string> variables, RobotMessageSender robot)
	{
		barrelRoller.Update();
		earthquaker.Update();

		// A simple example of how to have the Pi program display information from the Unity program.
		if(Input.GetKeyDown(KeyCode.A))
			robot.SendMessage("/print The A key was pressed in Unity.");
	}

	// This function is called every time a command is received from the robot.
	// It is always called in the same thread as the Update function.
	public void Command(RobotCommand command, IDictionary<string, string> variables, RobotMessageSender robot)
	{
		//BARREL ROLL
		if(command.Is("barrel roll"))
		{
			if(!barrelRoller.IsRolling())
			{
				robot.SendMessage("/say Weeeeee!!!");

				barrelRoller.SetSecondsPerRotation(4);
				barrelRoller.StartBarrelRoll();
			}
		}

		//EARTHQUAKE
		else if(command.Is("earthquake"))
		{
			if(!earthquaker.IsQuaking())
			{
				robot.SendMessage("/say EARTHQUAKE!!!");

				earthquaker.SetDurationInSeconds(5);
				earthquaker.SetMagnitude(2);
				earthquaker.StartEarthquake();
			}
		}


		//QUEST CONTROLS-----------------------------
		else if(command.Is("/add quest #s"))
			quests.Add(command.GetString(0));

		else if(command.Is("/update quest #i #s"))
			quests.Update(command.GetInteger(0), command.GetString(1));

		else if(command.Is("/close quest #i"))
			quests.Close(command.GetInteger(0));

		else if(command.Is("/open quest #i"))
			quests.Open(command.GetInteger(0));

		else if(command.Is("/remove quest #i"))
			quests.Remove(command.GetInteger(0));


		//MUSIC CONTROLS-----------------------------
		else if(command.Is ("/music on")) {
			jukeBox.getCommand = true;
			jukeBox.RobotEnableMusic = true;
			//robot.SendMessage("/say Music Toggled");
		} else if(command.Is("/music off")) {
			jukeBox.getCommand = true;
			jukeBox.RobotEnableMusic = false;
		} else if (command.Is ("/music mute")) {
			jukeBox.getCommand = true;
			jukeBox.RobotMute = true;
			//mute music
		} else if (command.Is("/music unmute")) {
			jukeBox.getCommand = true;
			jukeBox.RobotMute = false;
			//unmute music
		} else if (command.Is("/music next")) {
			jukeBox.getCommand = true;
			jukeBox.RobotSkipTrack = true;
			//skip to next track
		} else if (command.Is ("/music back")) {
			jukeBox.getCommand = true;
			jukeBox.RobotBackTrack = true;
			//skip to previous track
		} else if(command.Is("/music restart")) {
			jukeBox.getCommand = true;
			jukeBox.RobotRestartTrack = true;
			//restart the current track
		} else if (command.Is ("/music loop")) {
			jukeBox.getCommand = true;
			jukeBox.RobotLoopTrack = true;
			//loops the current song
		} else if (command.Is ("/music unloop")) {
			jukeBox.getCommand = true;
			jukeBox.RobotLoopTrack = false;
			//unloops the current track
		}

	}

	BarrelRoller barrelRoller = new BarrelRoller();
	EarthQuaker earthquaker = new EarthQuaker();
	Quests quests = new Quests();
}
