using System.Collections.Generic;

public class RobotStuff
{
	public void Update(IDictionary<string, string> variables)
	{
	}

	public void Command(RobotCommand command, IDictionary<string, string> variables)
	{
		if(command.commandDescription == "barrel roll")
		{
			// Rotate stuff.
		}
	}
}
