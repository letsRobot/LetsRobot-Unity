using System;
using System.Collections.Generic;

public struct RobotChatMessage
{
	public string user;
	public string message;
	public bool isCommand;
	public bool isExecuting;
}

public struct RobotCommand
{
	public string command;
	public string commandDescription;
}

class RobotMessages : RobotMessageReceiver
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

		if(messageType == "chat")
		{
			var user        = tokenizer.GetToken();
			var chatMessage = tokenizer.GetString();

			AddMessage(new InternalRobotMessage(user, chatMessage));
		}

		else if(messageType == "command")
		{
			var isFromChat             = tokenizer.GetToken() == "from_chat";
			var user                   = tokenizer.GetToken();
			var commandId              = Convert.ToInt32(tokenizer.GetToken());
			var commandDescriptionSize = Convert.ToInt32(tokenizer.GetToken());
			var commandDescription     = tokenizer.GetString(commandDescriptionSize);
			var command                = tokenizer.GetString();

			var internalRobotMessage = new InternalRobotMessage(user, command, commandDescription, commandId);

			AddCommand(internalRobotMessage);

			if(isFromChat)
				AddMessage(internalRobotMessage);
		}

		else if(messageType == "command_begin")
		{
			var commandId = Convert.ToInt32(tokenizer.GetToken());

			SetCommandIsExecuting(commandId, true);
		}

		else if(messageType == "command_end")
		{
			var commandId = Convert.ToInt32(tokenizer.GetToken());

			SetCommandIsExecuting(commandId, false);
		}

		else if(messageType == "variable")
		{
			var variable = tokenizer.GetToken();
			var value    = tokenizer.GetString();

			SetVariable(variable, value);
		}

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
			foreach(var message in chatMessages)
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
			return new Dictionary<string, string>(variables);
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

	void AddMessage(InternalRobotMessage message)
	{
		lock(chatMessagesLock)
		{
			chatMessages.Add(message);
		}

		TrimChatMessages();
	}

	void AddCommand(InternalRobotMessage internalRobotMessage)
	{
		lock(commandsLock)
		{
			var command                = new RobotCommand();
			command.command            = internalRobotMessage.message;
			command.commandDescription = internalRobotMessage.commandDescription;

			commands.Add(command);
		}
	}

	void SetCommandIsExecuting(int commandId, bool isExecuting)
	{
		lock(commandsLock)
		{
			for(int i = 0; i < chatMessages.Count; i++)
			{
				var message = chatMessages[i];
				
				if(message.commandId == commandId)
					message.isExecuting = isExecuting;
				else
					message.isExecuting = false;
				
				chatMessages[i] = message;
			}
		}
	}

	void SetVariable(string variable, string value)
	{
		lock(variablesLock)
		{
			variables[variable] = value;
		}
	}

	void TrimChatMessages()
	{
		lock(chatMessagesLock)
		{
			while(chatMessages.Count > maxMessageNumber)
				chatMessages.RemoveAt(0);
		}
	}

	struct InternalRobotMessage
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
	}

	RobotConnection connection;
	object chatMessagesLock                  = new object();
	object commandsLock                      = new object();
	object variablesLock                     = new object();
	int maxMessageNumber                     = 100;
	IList<InternalRobotMessage> chatMessages = new List<InternalRobotMessage>();
	IList<RobotCommand> commands             = new List<RobotCommand>();
	IDictionary<string, string> variables    = new Dictionary<string, string>();
}
