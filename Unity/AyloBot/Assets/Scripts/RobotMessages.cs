using System;
using System.Collections.Generic;
using SimpleJSON;

public struct RobotChatMessage
{
	public string user;
	public string message;
	public bool isCommand;
	public bool isExecuting;
}

public struct InternalRobotMessage
{
	public InternalRobotMessage(string user, string message)
	{
		this.user          = user;
		this.message       = message;
		commandDescription = "";
		commandId          = 0;
		isCommand          = false;
		isExecuting        = false;
		newMessage         = false;
	}
	
	public InternalRobotMessage(string user, string message, string commandDescription, int commandId)
	{
		this.user               = user;
		this.message            = message;
		this.commandDescription = commandDescription;
		this.commandId          = commandId;
		isCommand               = true;
		isExecuting             = false;
		newMessage              = false;
	}
	
	public string user;
	public string message;
	public string commandDescription;
	public int commandId;
	public bool isCommand;
	public bool isExecuting;
	public bool newMessage;
};

public class RobotMessages : RobotMessageReceiver, RobotMessageSender
{
	public RobotMessages(string server, int port)
	{
		connection = new RobotConnection(server, port, this);
	}

	public void SetServer(string server, int port)
	{
		connection.SetServer(server, port);
	}

	public void Stop()
	{
		connection.Stop();
	}
	
	public void NewMessage(string message)
	{
		Tokenizer tokenizer = new Tokenizer(message, ' ');

		var messageType = tokenizer.GetToken();
		//UnityEngine.Debug.Log (messageType);

		if (messageType == "chat") {
			var user = tokenizer.GetToken ();
			var chatMessage = tokenizer.GetString ();

			AddMessage (new InternalRobotMessage (user, chatMessage));
		} else if (messageType == "command") {
			var isFromChat = tokenizer.GetToken () == "from_chat";
			var user = tokenizer.GetToken ();
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());
			var commandDescriptionSize = Convert.ToInt32 (tokenizer.GetToken ());
			var commandDescription = tokenizer.GetString (commandDescriptionSize);
			var command = tokenizer.GetString ();

			var internalRobotMessage = new InternalRobotMessage (user, command, commandDescription, commandId);

			AddCommand (internalRobotMessage);

			if (isFromChat)
				AddMessage (internalRobotMessage);
		} else if (messageType == "command_begin") {
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());

			SetCommandIsExecuting (commandId, true);
		} else if (messageType == "command_end") {
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());

			SetCommandIsExecuting (commandId, false);
		} else if (messageType == "variable") {
			var variable = tokenizer.GetToken ();
			var value = tokenizer.GetString ();

			SetVariable (variable, value);
		} else if (messageType == "parse") { 
			//UnityEngine.Debug.Log ("parsing");
			var from = tokenizer.GetToken();
			var privilegeLevel = Convert.ToInt32(tokenizer.GetToken());
			var platform = tokenizer.GetToken();
			var msg = tokenizer.GetString();
			//UnityEngine.Debug.Log (from);
			//UnityEngine.Debug.Log (privilegeLevel);
			//UnityEngine.Debug.Log (platform);
			//UnityEngine.Debug.Log (msg);
			//UnityEngine.Debug.Log ("done parsing");
		}
		else if(messageType == "run")
		{ 
			//UnityEngine.Debug.Log ("run");
			var json = tokenizer.GetString ();
			//Debug.Log(json);
			var packet=JSON.Parse(json);
			// packet["type"] = motor, led, sendCommandMessage
			// packet["from"] = user that typed it
			// packet["text"] = what they typed
			// packet["i"] = which led
			// packet["r"] = red
			// packet["g"] = green
			// packet["b"] = blue
			//UnityEngine.Debug.Log(packet["type"]);
			//UnityEngine.Debug.Log(packet["text"]);
			//UnityEngine.Debug.Log ("done run");
		}
		else if(messageType == "hello")
		{ }

		else
			throw new Exception();
	}

	public void SetMaximumNumberOfMessages(int maxMessageNumber)
	{
		this.maxMessageNumber = maxMessageNumber;

		TrimChatMessages();
	}

	public IList<RobotChatMessage> GetChatMessages()
	{
		var copyOfChatMessages = new List<RobotChatMessage>();

		lock(chatMessagesLock)
		{
			foreach(var message in Constants.roboStuff.chatMessages)
			{
				var chatMessage         = new RobotChatMessage();
				chatMessage.user        = message.user;
				chatMessage.message     = message.message;
				chatMessage.isCommand   = message.isCommand;
				chatMessage.isExecuting = message.isExecuting;

				copyOfChatMessages.Add(chatMessage);
			}
		}

		return copyOfChatMessages;
	}

	public IDictionary<string, string> GetVariables()
	{
		lock(variablesLock)
		{
			//return new Dictionary<string, string>(variables);
			return new Dictionary<string, string>(Constants.roboStuff.variables);
		}
	}

	public IList<RobotCommand> GetCommands()
	{
		lock(commandsLock)
		{
			IList<RobotCommand> returnCommands = commands;
			commands = new List<RobotCommand>();
			return returnCommands;
		}
	}

	public void SendMessage(string message)
	{
		connection.SendMessage(message);
	}

	void AddMessage(InternalRobotMessage message)
	{
		lock(chatMessagesLock)
		{
			Constants.roboStuff.chatMessages.Add(message);
		}

		TrimChatMessages();
	}

	void AddCommand(InternalRobotMessage internalRobotMessage)
	{
		lock(commandsLock)
		{
			var command = new RobotCommand(internalRobotMessage.commandDescription, internalRobotMessage.message);
			commands.Add(command);
		}
	}

	void SetCommandIsExecuting(int commandId, bool isExecuting)
	{
		lock(commandsLock)
		{
			for(int i = 0; i < Constants.roboStuff.chatMessages.Count; i++)
			{
				var message = Constants.roboStuff.chatMessages[i];
				
				if(message.commandId == commandId)
					message.isExecuting = isExecuting;
				else
					message.isExecuting = false;
				
				Constants.roboStuff.chatMessages[i] = message;
			}
		}
	}

	void SetVariable(string variable, string value)
	{
		lock(variablesLock)
		{
			//variables[variable] = value;
			Constants.roboStuff.variables[variable]=value;
		}
	}

	void TrimChatMessages()
	{
		lock(chatMessagesLock)
		{
			while(Constants.roboStuff.chatMessages.Count > maxMessageNumber)
				Constants.roboStuff.chatMessages.RemoveAt(0);
		}
	}

	RobotConnection connection;
	object chatMessagesLock                  = new object();
	object commandsLock                      = new object();
	object variablesLock                     = new object();
	int maxMessageNumber                     = 100;
	IList<RobotCommand> commands             = new List<RobotCommand>();
	// 20160603 rtharp
	// moved out to RobotStuff, so we can have multiple connecctions
	// but only one set of chat & variable
	// since we only have the one display of them
	//IList<InternalRobotMessage> chatMessages = new List<InternalRobotMessage>();
	//IDictionary<string, string> variables    = new Dictionary<string, string>();
}
