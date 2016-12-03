using System;
using System.Collections.Generic;
using SimpleJSON;


//Basic structure for elements containing a chat message
public struct RobotChatMessage {

	public string user; //user will eventually need user type
	public string message; //message is the text input from the user, unless it's a command.
	//TODO: messages should all be messages, except we assign a message type to them in the future.
	public bool isCommand; //Is this message a command? 
	public bool isExecuting; //Is it executing?
}

//Message parameters are defined and set here to determine how they are displayed on the GUI
public struct InternalRobotMessage {


	//Set these values for a normal chat message.
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
	
	//Set these values if the message is a command
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
	
	//These values are set by InternalRobotMessage method above
	public string user;
	public string message;
	public string commandDescription;
	public int commandId;
	public bool isCommand;
	public bool isExecuting;
	public bool newMessage;
};

//Gets messages from another source over the network via TCP Sockets
//Class uses RobotMessageReceiver & RobotMEssageSender interfaces from RobotConnection.cs
public class RobotMessages : RobotMessageReceiver, RobotMessageSender
{

	RobotConnection connection;
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
	
	//Message package is assembled from string
	public void NewMessage(string message)
	{
		Tokenizer tokenizer = new Tokenizer(message, ' ');

		//message type is given from the broadcast source, 
		//The broadcast source must contain a particular key / string pair for it to be picked up here.
		var messageType = tokenizer.GetToken();
		//UnityEngine.Debug.Log (messageType);
		

		if (messageType == "chat") {
			var user = tokenizer.GetToken ();
			var chatMessage = tokenizer.GetString ();

			//Basic chat message generation, user + message
			AddMessage (new InternalRobotMessage (user, chatMessage));

			//command message generation, user + command, commandDescription, commandID
		} else if (messageType == "command") {
			var isFromChat = tokenizer.GetToken () == "from_chat";
			var user = tokenizer.GetToken ();
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());
			var commandDescriptionSize = Convert.ToInt32 (tokenizer.GetToken ());
			var commandDescription = tokenizer.GetString (commandDescriptionSize);
			var command = tokenizer.GetString ();

			var internalRobotMessage = new InternalRobotMessage (user, command, commandDescription, commandId);

			AddCommand (internalRobotMessage);

			//Admin messages are hidden from the GUI
			if (isFromChat)
				AddMessage (internalRobotMessage);

			//handles command execution state for GUI	
		} else if (messageType == "command_begin") {
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());

			SetCommandIsExecuting (commandId, true);
		} else if (messageType == "command_end") {
			var commandId = Convert.ToInt32 (tokenizer.GetToken ());

			SetCommandIsExecuting (commandId, false);

			//Hook for setting a global variable, this is a secure message type that should only come from an admin
		} else if (messageType == "variable") {
			var variable = tokenizer.GetToken ();
			var value = tokenizer.GetString ();

			SetVariable (variable, value);

			//This message type is a hack for Skynet for generating new message types.
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

		//Not exactly what this does, this is a Ryan thing. - Jill
		else if(messageType == "run")
		{ 
			//UnityEngine.Debug.Log ("run");
			var json = tokenizer.GetString ();
			//Debug.Log(json);
			var packet =JSON.Parse(json);
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
		//If no message types are found... 
			throw new Exception();
	}

	int maxMessageNumber = 100;
	public void SetMaximumNumberOfMessages(int maxMessageNumber)
	{
		this.maxMessageNumber = maxMessageNumber;

		TrimChatMessages();
	}

	//Package up the components of the message into a List
	//TODO: This should not be directly referencing Constants, this class should be as encapsulated as possible.

	object chatMessagesLock = new object();
	public IList<RobotChatMessage> GetChatMessages()
	{
		var copyOfChatMessages = new List<RobotChatMessage>();

		//Note to self, Lock ensures that chatmessages cannot be updated by a separate thread while being
		//executed by another thread.
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

	//Get specific variables from broadcast source related to the robot.
	object variablesLock = new object();
	public IDictionary<string, string> GetVariables()
	{
		lock(variablesLock)
		{
			//return new Dictionary<string, string>(variables);
			return new Dictionary<string, string>(Constants.roboStuff.variables);
		}
	}

	IList<RobotCommand> commands = new List<RobotCommand>();
	object commandsLock  = new object();

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

	
	// 20160603 rtharp
	// moved out to RobotStuff, so we can have multiple connecctions
	// but only one set of chat & variable
	// since we only have the one display of them
	//IList<InternalRobotMessage> chatMessages = new List<InternalRobotMessage>();
	//IDictionary<string, string> variables    = new Dictionary<string, string>();
}
