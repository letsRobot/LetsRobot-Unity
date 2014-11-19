using System.Collections.Generic;

public class RobotStuff
{
	// This function is called before every frame is shown.
	public void Update(IDictionary<string, string> variables, RobotMessageSender robot)
	{
	}

	// This function is called every time a command is received from the robot.
	public void Command(RobotCommand command, IDictionary<string, string> variables, RobotMessageSender robot)
	{
		if(command.commandDescription == "barrel roll")
		{
			//robot.SendMessage("/say Weeeeee!!!");

			// Rotate stuff.
		}
	}
}
