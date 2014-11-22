using UnityEngine;
using System.Collections.Generic;

public class RobotStuff
{
	// This function is called before every frame is shown.
	public void Update(IDictionary<string, string> variables, RobotMessageSender robot)
	{
		barrelRoller.Update();
		earthquaker.Update();
	}

	// This function is called every time a command is received from the robot.
	// It is always called in the same thread as the Update function.
	public void Command(RobotCommand command, IDictionary<string, string> variables, RobotMessageSender robot)
	{
		if(command.Is("barrel roll"))
		{
			if(!barrelRoller.IsRolling())
			{
				robot.SendMessage("/say Weeeeee!!!");

				barrelRoller.SetSecondsPerRotation(4);
				barrelRoller.StartBarrelRoll();
			}
		}

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
	}

	BarrelRoller barrelRoller = new BarrelRoller();
	EarthQuaker earthquaker = new EarthQuaker();
	Quests quests = new Quests();
}
