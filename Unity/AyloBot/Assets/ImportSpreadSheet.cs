using UnityEngine;
using System.Collections;
using System.IO;
using System.Collections.Generic;

//This class imports the story data spread sheet, and also sort of manages the HUD appearance.
//I should have named it something more appropriate : D
public class ImportSpreadSheet : MonoBehaviour {

	public TextAsset StoryData;
	string ReadSheet;
	string GetLine;
	public int PrintThisLine;
	int CheckLine;

	//Variables that control the Story Display on the HUD
	public GameObject myName;
	public GameObject myText;
	public GameObject myPortrait;
	TextMesh displayName;
	TextMesh displayText;
	string characterName;
	string characterText;

	//Reference script that controls the character
	CharacterManager manageCharacter;

	bool wrongLine;

	//Which values are located at which tabs
	int tagTab = 0; //tag associated with scene
	int nameTab = 1; //Character Name
	int emoteTab = 2; //Character Emote
	int lineTab = 3; //Character's Line
	int timerTab = 4; //How long to play the scene for

	public static float sceneTime;
	float getTime;
	public float customTime = 4.5f;


	//These variables manage a sequence of cut-scenes, 
	//aka: characters having conversation or multiple dialogues
	bool indexSequence = false; //triggers indexing lines for a cut-scene sequence
	bool playSequence = false; //returns true if a cut-scene sequence is ready to play
	public string getSequence = "";
	string checkSequence = "";
	List<int> cueLines; //line numbers associated with a sequence
	//int spreadSheetIndex; //number of lines on the spread sheet.




	int lineCount;

	public static string cueRobot = "";
	public bool debugPlayer;

	void cueSequence () {

		if (debugPlayer == false) {
			getSequence = cueRobot;
		}
		//Play a set of lines all with the same tags
		if (checkSequence != getSequence) {

			//Debug.Log("Check Sequence = " + checkSequence);
			indexSequence = true;
			lineCount = 0;

			cueLines = new List<int>();
			sequenceStep = 0;

			for (int i = 0; i < FetchLine; i++) {
				lineCount = i;
				if (getString(tagTab) == getSequence) {
					cueLines.Add(i);
				}
			}
			foreach (int derp in cueLines) {
				//Debug.Log(getSequence + " At Line: " + derp);
			} 

			if (cueLines.Count != 0) {
				playSequence = true;
				//Debug.Log("Play sequence is now true");
			} 
			indexSequence = false;
			checkSequence = getSequence;
		}
		runSequence();
	}

	public static bool sendLine = false;
	int sequenceStep = 0;

	int trackThing = 0;
	void runSequence() {

		//Debug.Log("Run Sequence is Updating");
		if (sendLine == false) {
			//Debug.Log("Send Line is False");
		}
		if (playSequence == true && sendLine == false) {
			sendLine = true;
			trackThing++;
			//Debug.Log("Step: " + trackThing + " Cue Lines Count: " + cueLines.Count + " Sequence step: " + sequenceStep);

			if (sequenceStep < cueLines.Count) {
				PrintThisLine = cueLines[sequenceStep];
				//Debug.Log("Sending line: " + sequenceStep);
				sequenceStep++;
			} else {
				playSequence = false;
				sendLine = false;
				//Debug.Log("Play Sequence now over");
			}
		}
		//Debug.Log("Sequence Step: " + sequenceStep);
	}

	void Awake () {

		wrongLine = false;
		ReadSheet = StoryData.text;
		initializeStory();
		//initializeParse();
		//Debug.Log("Number of Lines: " + LineIndex);
		PrintThisLine = 0;
		LineGetter(0);
		indexSequence = false;
		playSequence = false;
		sendLine = false;
	}

	public int lineLength = 5;
	// Wrap text by line height
	private string textWrap(string input, int lineLength){
		
		// Split string by char " "         
		string[] words = input.Split(" "[0]);
		
		// Prepare result
		string result = "";
		
		// Temp line string
		string line = "";
		
		// for each all words        
		foreach(string s in words){
			// Append current word into line
			string temp = line + " " + s;
			
			// If line length is bigger than lineLength
			if(temp.Length > lineLength){
				
				// Append current line into result
				result += line + "\n";
				// Remain word append into new line
				line = s;
			}
			// Append current word into current line
			else {
				line = temp;
			}
		}
		
		// Append last line into result        
		result += line;
		
		// Remove first " " char
		return result.Substring(1,result.Length-1);
	}

	public static bool sceneFromRobot = false;
	public static Characters charFromRobot = Characters.UNKNOWN;
	public static Emotes emoteFromRobot = Emotes.DEFAULT;
	public static string nameFromRobot = "UNKNOWN";
	public static string lineFromRobot = "Transmission to robot interupted";
	
	// Update is called once per frame
	void Update () {

		cueSequence();


		//Make sure the line we print is in range to avoid errors.
		if (CheckLine != PrintThisLine && PrintThisLine != 0 && PrintThisLine < FetchLine || sceneFromRobot == true) {

			string storeName = "Default";
			string prepLine = "Transmission Interupted";
			Emotes parseEmote = Emotes.DEFAULT;
			Characters parseCharacter = Characters.UNKNOWN;
			getTime = customTime;

			if (sceneFromRobot == true) {

				storeName = nameFromRobot;
				prepLine = lineFromRobot;
				parseEmote = emoteFromRobot;
				parseCharacter = charFromRobot;
				sceneFromRobot = false;


			} else {
				storeName = getString(nameTab);
				prepLine = getString(lineTab);
				parseEmote = (Emotes)System.Enum.Parse(typeof(Emotes), getString (emoteTab));
				parseCharacter = (Characters)System.Enum.Parse (typeof(Characters), storeName);
				string storeFloat = getString(timerTab);
				if (storeFloat != "") {
					getTime = float.Parse(storeFloat);
				} else {
					getTime = customTime;
				}

			}

			//displayText.text = getString(lineTab);
			displayText.text = textWrap(prepLine, lineLength);
			displayName.text = storeName;
			//Convert the string StoreName into a Characters Enum type




			manageCharacter.character = parseCharacter;
			manageCharacter.emote = parseEmote;
			sceneTime = getTime;
			//Debug.Log("Scene Time: " + getTime);
			//Tell the character manager to go ahead and display the character
			CharacterManager.playScene = true;
	
			CheckLine = PrintThisLine;
		}

		if (CharacterManager.sceneActive != true && CharacterManager.playScene == false) {
			displayText.text = "";
			displayName.text = "";
		}
	}

	//References objects needed to display character scene
	void initializeStory () {

		//References to HUD objects
		displayName = myName.gameObject.GetComponent<TextMesh>();
		displayText = myText.gameObject.GetComponent<TextMesh>();
		manageCharacter = myPortrait.gameObject.GetComponent<CharacterManager>();
	}

	//Get's The string from the specified line and tab in StoryData
	string getString (int tabNum) {
		string whichString = fetchTab(tabNum);
		return whichString;
	}

	//Gets the desired tab from any particular line in StoryData
	string[] indexTabs;
	string fetchTab (int whichTab) {

		if (indexSequence == true) {
			//if checking lines for a cut-scene, do this!
			indexTabs = LineGetter(lineCount).Split("\t"[0]);
		} else { 
			//Fetching individual lines
			indexTabs = LineGetter(PrintThisLine).Split("\t"[0]);
		}
		//Debug.Log("Number of tabs: " + indexTabs.Length);

		if (wrongLine == true) {
			string oops = "Zomg Internal error! We are going down in flames!";
			return oops;
		} else {
			return indexTabs[whichTab];
		}

	}
	
	int FetchLine = 1; //Start at 1 so we can pretend line 0 doesn't exist!
	//Indexes and fetches the requested line from the StoryData Sheet
	string LineGetter (int WhichLine) {
		string[] indexLines = ReadSheet.Split("\n"[0]);
		FetchLine = indexLines.Length;

		for (int i = 0; i < FetchLine; i++) {
			//string PrintStrings = indexLines[i];
			//Debug.Log("Line " + i + " :" + PrintStrings);
		}

		if (WhichLine >= indexLines.Length) {
			WhichLine = 0;
			wrongLine = true;
		} else {
			wrongLine = false;
		}
			return indexLines[WhichLine];
	}
}
