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
		richText += ": ";
		richText += message.message;
		
		richText += "</color>";
		richText += "\n";

		return richText;
	}

	RobotMessages robotMessages;
}
