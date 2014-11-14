using System;
using System.Collections.Generic;

public struct RobotChatMessage
{
	public string user;
	public string message;
	public bool isCommand;
	public bool isExecuting;
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

			AddMessage(new InternalRobotMessage(user, command, commandDescription, commandId, isFromChat));
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

			lock(messageLock)
			{
				variables[variable] = value;
			}
		}

		else
			throw new Exception();
	}

	public void SetMaximumNumberOfMessages(int maxMessageNumber)
	{
		this.maxMessageNumber = maxMessageNumber;

		TrimMessages();
	}

	public IList<RobotChatMessage> GetChatMessages()
	{
		var chatMessages = new List<RobotChatMessage>();

		lock(messageLock)
		{
			foreach(var message in messages)
			{
				if(!message.isFromChat)
					continue;

				RobotChatMessage chatMessage = new RobotChatMessage();
				chatMessage.user        = message.user;
				chatMessage.message     = message.message;
				chatMessage.isCommand   = message.isCommand;
				chatMessage.isExecuting = message.isExecuting;

				chatMessages.Add(chatMessage);
			}
		}

		return chatMessages;
	}

	public string GetVariable(string variable)
	{
		lock(messageLock)
		{
			return variables[variable];
		}
	}

	void AddMessage(InternalRobotMessage message)
	{
		lock(messageLock)
		{
			messages.Add(message);
		}

		TrimMessages();
	}

	void SetCommandIsExecuting(int commandId, bool isExecuting)
	{
		lock(messageLock)
		{
			for(int i = 0; i < messages.Count; i++)
			{
				var message = messages[i];

				if(message.commandId == commandId)
					message.isExecuting = isExecuting;
				else
					message.isExecuting = false;

				messages[i] = message;
			}
		}
	}

	void TrimMessages()
	{
		lock(messageLock)
		{
			while(messages.Count > maxMessageNumber)
				messages.RemoveAt(0);
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
			isFromChat         = true;
			isExecuting        = false;
			newMessage         = false;
		}

		public InternalRobotMessage(string user, string message, string commandDescription, int commandId, bool isFromChat)
		{
			this.user               = user;
			this.message            = message;
			this.commandDescription = commandDescription;
			this.commandId          = commandId;
			isCommand               = true;
			this.isFromChat         = isFromChat;
			isExecuting             = false;
			newMessage              = false;
		}

		public string user;
		public string message;
		public string commandDescription;
		public int commandId;
		public bool isCommand;
		public bool isFromChat;
		public bool isExecuting;
		public bool newMessage;
	}

	RobotConnection connection;
	object messageLock = new object();
	IList<InternalRobotMessage> messages = new List<InternalRobotMessage>();
	int maxMessageNumber = 100;
	IDictionary<string, string> variables = new Dictionary<string, string>();
}
