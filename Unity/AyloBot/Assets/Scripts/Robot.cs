using System;
using System.Collections.Generic;
using UnityEngine;

public class Robot : MonoBehaviour
{
	public string server;
	public int port;
	public int numberOfChatMessages;
	public string chatUsernameColor;
	public string normalChatMessageColor;
	public string commandColor;
	public string executingCommandColor;
	public string openQuestColor;
	public string closedQuestColor;

	void Start()
	{
		server = Constants.IP1;
		robotMessages = new RobotMessages(server, port);
		robotStuff = new RobotStuff();
	}

	public void OnApplicationQuit()
	{
		robotMessages.Stop();
	}

	public void Update()
	{
		robotMessages.SetServer(server, port);

		variables = robotMessages.GetVariables();
		robotStuff.Update(variables, robotMessages);
		DispatchCommands();
		UpdateChat();
		UpdateHud();
	}

	void DispatchCommands()
	{
		foreach(var command in robotMessages.GetCommands())
			robotStuff.Command(command, variables, robotMessages);
	}

	void UpdateChat()
	{
		robotMessages.SetMaximumNumberOfMessages(numberOfChatMessages);
		var chatMessages = robotMessages.GetChatMessages();

		var chat = GameObject.Find("Chat").GetComponent<TextMesh>();
		chat.text = "";

		foreach(var message in chatMessages)
		{
			if(message.user == "jtv") // Ignore messages from Twitch itself.
				continue;

			chat.text += ChatMessageToRichTextLine(message);
		}
	}

	string ChatMessageToRichTextLine(RobotChatMessage message)
	{
		string richText = "";

		string color = normalChatMessageColor;

		if(message.isCommand)
			color = commandColor;

		if(message.isExecuting)
			color = executingCommandColor;

		var isAction = message.message.StartsWith(chatActionPrefix) && message.message.EndsWith(chatActionPostfix);

		richText += "<color=#" + chatUsernameColor + ">";
		richText += message.user;
		richText += isAction ? " " : ": ";
		richText += "</color>";

		richText += "<color=#" + color + ">";

		if(isAction)
			richText += message.message.Substring(chatActionPrefix.Length, message.message.Length - chatActionPrefix.Length - 1);
		else
			richText += message.message;

		richText += "</color>";
		richText += "\n";

		return richText;
	}

	void UpdateHud()
	{
		UpdateLeds();
		UpdateEcho();
	}

	void UpdateLeds()
	{
		for(int iLed = 0; iLed < 16; iLed++)
		{
			try
			{
				string ledVariable = "led_" + iLed;
				string ledValue = variables[ledVariable];

				Tokenizer ledValueTokenizer = new Tokenizer(ledValue, ' ');

				var r = Convert.ToInt32(ledValueTokenizer.GetToken());
				var g = Convert.ToInt32(ledValueTokenizer.GetToken());
				var b = Convert.ToInt32(ledValueTokenizer.GetToken());

				UpdateLed(iLed, r / 255.0f, g / 255.0f, b / 255.0f);
			}
			catch(KeyNotFoundException)
			{ }
		}
	}

	void UpdateLed(int iLed, float r, float g, float b)
	{
		var led = GameObject.Find("LED" + iLed).GetComponent<MeshRenderer>();
		led.material.color = new Color(r, g, b);
	}

	void UpdateEcho()
	{
		try
		{
			var echoCm = Convert.ToInt32(variables["echo"]);
			var echo = GameObject.Find("Echo").GetComponent<TextMesh>();
			echo.text = "" + echoCm + " cm";
		}
		catch(KeyNotFoundException)
		{
			// If the "echo" variable has not been set we do not update the text .
		}
	}

	RobotMessages robotMessages;
	RobotStuff robotStuff;
	IDictionary<string, string> variables;
	string chatActionPrefix = "\x0001ACTION"; // This is what Twitch puts before and after a /me message.
	string chatActionPostfix = "\x0001";
}
