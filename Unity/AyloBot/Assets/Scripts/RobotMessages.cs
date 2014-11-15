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

			var internalRobotMessage = new InternalRobotMessage(user, command, commandDescription, commandId);

			lock(messageLock)
			{
				commands.Add(internalRobotMessage);
			}

			if(isFromChat)
				AddMessage(new InternalRobotMessage(user, command, commandDescription, commandId));
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

		TrimChatMessages();
	}

	public IList<RobotChatMessage> GetChatMessages()
	{
		var returnChatMessages = new List<RobotChatMessage>();

		lock(messageLock)
		{
			foreach(var message in chatMessages)
			{
				RobotChatMessage chatMessage = new RobotChatMessage();
				chatMessage.user        = message.user;
				chatMessage.message     = message.message;
				chatMessage.isCommand   = message.isCommand;
				chatMessage.isExecuting = message.isExecuting;

				returnChatMessages.Add(chatMessage);
			}
		}

		return returnChatMessages;
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
			chatMessages.Add(message);
		}

		TrimChatMessages();
	}

	void SetCommandIsExecuting(int commandId, bool isExecuting)
	{
		lock(messageLock)
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

	void TrimChatMessages()
	{
		lock(messageLock)
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
	object messageLock = new object();
	int maxMessageNumber = 100;
	IList<InternalRobotMessage> chatMessages = new List<InternalRobotMessage>();
	IList<InternalRobotMessage> commands = new List<InternalRobotMessage>();
	IDictionary<string, string> variables = new Dictionary<string, string>();
}
