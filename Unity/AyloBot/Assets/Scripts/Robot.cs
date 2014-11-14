using System;
using System.Collections.Generic;
using UnityEngine;

public class Robot : MonoBehaviour
{
	public string server;
	public int port;
	public int numberOfChatMessages;
	public string normalChatMessageColor;
	public string commandColor;
	public string executingCommandColor;

	void Start()
	{
		server = Constants.IP1;
		robotMessages = new RobotMessages(server, port);
	}

	public void OnApplicationQuit()
	{
		robotMessages.Stop();
	}

	public void Update()
	{
		robotMessages.SetServer(server, port);

		UpdateChat();
		UpdateHud();
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

		richText += "<color=" + color + ">";
		richText += message.user;

		if(message.message.StartsWith(chatActionPrefix) && message.message.EndsWith(chatActionPostfix))
		{
			richText += " ";
			richText += message.message.Substring(chatActionPrefix.Length, message.message.Length - chatActionPrefix.Length - 1);
		}
		else
		{
			richText += ": ";
			richText += message.message;
		}

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
		for(int i = 0; i < 16; i++)
		{
			try
			{
				string ledVariable = "led_" + i;
				string ledValue = robotMessages.GetVariable(ledVariable);

				Tokenizer ledValueTokenizer = new Tokenizer(ledValue, ' ');

				var r = Convert.ToInt32(ledValueTokenizer.GetToken());
				var g = Convert.ToInt32(ledValueTokenizer.GetToken());
				var b = Convert.ToInt32(ledValueTokenizer.GetToken());

				UpdateLed(i, r / 255.0f, g / 255.0f, b / 255.0f);
			}
			catch(KeyNotFoundException)
			{ }
		}
	}

	void UpdateLed(int led, float r, float g, float b)
	{
	}

	void UpdateEcho()
	{
		var echo = GameObject.Find("Echo").GetComponent<TextMesh>();
		int echoCm = 0;

		try
		{
			echoCm = Convert.ToInt32(robotMessages.GetVariable("echo"));
		}
		catch(KeyNotFoundException)
		{ }

		echo.text = "" + echoCm + " cm";
	}

	RobotMessages robotMessages;
	string chatActionPrefix = "\x0001ACTION";
	string chatActionPostfix = "\x0001";
}
