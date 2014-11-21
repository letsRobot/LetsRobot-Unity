using System;
using System.Collections.Generic;

public class RobotCommand
{
	public RobotCommand(string commandDescription, string actualCommand)
	{
		this.actualCommand = actualCommand;
		this.commandDescription = commandDescription;

		BuildActualParameterList();
	}

	public bool Is(string commandDescription)
	{
		return this.commandDescription == commandDescription;
	}

	public int GetInteger(int i)
	{
		return Convert.ToInt32(actualParameters[i]);
   }

	public string GetWord(int i)
	{
		return actualParameters[i];
	}

	public string GetString(int i)
	{
		return actualParameters[i];
   }

	void BuildActualParameterList()
	{
		Tokenizer commandDescriptionTokenizer = new Tokenizer(commandDescription, ' ');
		Tokenizer actualCommandTokenizer = new Tokenizer(actualCommand, ' ');
      
		while(actualCommandTokenizer.HasMore())
		{
			var formalParameter = commandDescriptionTokenizer.GetToken();
         
			if(formalParameter == "#i" || formalParameter == "#w")
				actualParameters.Add(actualCommandTokenizer.GetToken());

			else if(formalParameter == "#s")
				actualParameters.Add(actualCommandTokenizer.GetString());

			else
				actualCommandTokenizer.GetToken(); // The token is a keyword and we ignore it.
		}
         
	}

	string actualCommand;
	string commandDescription;
	IList<string> actualParameters = new List<string>();
}

