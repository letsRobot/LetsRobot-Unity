using System;
using System.Collections.Generic;
using UnityEngine;
using System.Text.RegularExpressions;

public class Robot : MonoBehaviour
{
	public string server;
	public int port;
	public int numberOfChatMessages;
	public string chatUsernameColor;
	public string[] chatUserNameColors;
	public string normalChatMessageColor;
	public string commandColor;
	public string executingCommandColor;
	public string openQuestColor;
	public string closedQuestColor;
	public bool Telly;

	int firstLED;
	int lastLED;

	//dictionary for tracking users and assigning them colors in chat
	Dictionary<string, string> users = new Dictionary<string, string>();
	
	void Start()
	{
		startChat();

		server = Constants.IP1;
		robotMessages = new RobotMessages(server, port);
		robotStuff = Constants.roboStuff;
	
		if (Telly == true) {
			firstLED = 1;
			lastLED = 19;
		} else {
			firstLED = 0;
			lastLED = 16;
		}
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

	public IDictionary<string, string> getIMUVariables() {
		return variables;
	}

	void DispatchCommands()
	{
		foreach(var command in robotMessages.GetCommands())
			robotStuff.Command(command, variables, robotMessages);
	}

	//CHAT STUFFS ------------------------------------------------
	public GameObject showChat;
	TextMesh robotChat;
	Renderer renderChat;

	//Initialize chat Stuffs
	void startChat () {

		robotChat = showChat.gameObject.GetComponent<TextMesh>();
		renderChat = showChat.gameObject.GetComponent<Renderer>();

	}

	void UpdateChat() //This updates the chat every frame (not ideal)
	{

	
		//Toggle Chat on and Off
		if (Constants.triggerChat == true && Constants.showChat == true) {
			renderChat.enabled = false;
			Constants.showChat = false;
			Constants.triggerChat = false;
		} else if (Constants.triggerChat == true && Constants.showChat == false) {
			renderChat.enabled = true;
			Constants.showChat = true;
			Constants.triggerChat = false;
		}

		//Show Commands Only
		if (Constants.triggerCommandOnly == true && Constants.commandsOnly == false) {
			Constants.triggerCommandOnly = false;
			Constants.commandsOnly = true;
		} else if (Constants.triggerCommandOnly == true && Constants.commandsOnly == true) {
			Constants.triggerCommandOnly = false;
			Constants.commandsOnly = false;
		}

		var numberCommandChatMessages = 0;
		var countMessages = robotMessages.GetChatMessages();
		robotChat.text = "";
		foreach(var message in countMessages)
		{
			numberCommandChatMessages += (message.user == "jtv" || (message.isCommand == false && Constants.commandsOnly == true) ? 1 : 0);
			//Debug.Log (numberCommandChatMessages);
		}

		if(Constants.commandsOnly) {
				robotMessages.SetMaximumNumberOfMessages(numberCommandChatMessages + numberOfChatMessages);
			} else {
				robotMessages.SetMaximumNumberOfMessages(numberOfChatMessages);
			}
		var chatMessages = robotMessages.GetChatMessages();

		foreach(var message in chatMessages)
		{
			if(message.user == "jtv" || message.isCommand == false && Constants.commandsOnly == true) // Ignore messages from Twitch itself.
			//if(message.user == "jtv") // Ignore messages from Twitch itself.
			{
				continue;
			}
				
			robotChat.text += ChatMessageToRichTextLine(message);
		}
	}



	string ChatMessageToRichTextLine(RobotChatMessage message)

	{
		string richText = "";
		string nonRichTextString = Regex.Replace(message.message, "<.*?>", string.Empty);
		string color = normalChatMessageColor;

		//Check to see if message is a command.
		if(message.isCommand)
			color = commandColor;

		if(message.isExecuting)
			color = executingCommandColor;

		var isAction = message.message.StartsWith(chatActionPrefix) && message.message.EndsWith(chatActionPostfix);

		chatUsernameColor = trackUsers(message.user);
		//Debug.Log("User Color: " + chatUsernameColor);
		richText += "<color=#" + chatUsernameColor + ">";
		richText += message.user;
		richText += isAction ? " " : ": ";
		richText += "</color>";

		richText += "<color=#" + color + ">";

		if(isAction)
			richText += nonRichTextString.Substring(chatActionPrefix.Length, nonRichTextString.Length - chatActionPrefix.Length - 1);
		else
			richText += nonRichTextString;

		richText += "</color>";
		richText += "\n";

		return richText;
	}

	//checks each user against the current dictionary,
	//If the user isn't in the Dictionary, then add them and assign a color to them
	string trackUsers(string checkUser) {
		
		if (!users.ContainsKey(checkUser)) {
			
			int randomColor = UnityEngine.Random.Range(0, chatUserNameColors.Length);
			chatUsernameColor = chatUserNameColors[randomColor];
			users.Add(checkUser, chatUsernameColor);
			//Debug.Log("Adding new user: " + checkUser);
			
		}
		
		string thisUserColor = "";
		users.TryGetValue(checkUser, out thisUserColor);
		//Debug.Log("Returning Color: " + thisUserColor);
		return thisUserColor;
		
	}

	//TODO: GetComponent should not be called during the update loop, 
	//an instance of the LED's needs to be created first or a reference to the LEDs.
	void UpdateHud()
	{
		UpdateLeds();
		UpdateEcho();
	}
	
	void UpdateLeds()
	{
		//Debug.Log ("Using Telly's Custom things, have to change values back to 0 & 16 for normal bots");
		for(int iLed = firstLED; iLed < lastLED; iLed++)
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

		//For Each LED object found with this index, change the color.
		var led = GameObject.Find("LED" + iLed).GetComponent<MeshRenderer>();
		var simLed = GameObject.Find ("sim" + iLed).GetComponent<MeshRenderer> ();
		led.material.color = new Color(r, g, b);
		simLed.material.color = new Color (r, g, b);
		
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
