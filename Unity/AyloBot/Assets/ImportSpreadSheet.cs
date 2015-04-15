using UnityEngine;
using System.Collections;
using System.IO;

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
	int nameTab = 1; //Character Name
	int emoteTab = 2; //Character Emote
	int lineTab = 3; //Character's Line


	void Awake () {

		wrongLine = false;
		ReadSheet = StoryData.text;
		initializeStory();
		//Debug.Log("Number of Lines: " + LineIndex);
		PrintThisLine = 0;
		LineGetter(0);
	}

	// Update is called once per frame
	void Update () {

		//Make sure the line we print is in range to avoid errors.
		if (CheckLine != PrintThisLine && PrintThisLine != 0 && PrintThisLine < FetchLine) {
			displayText.text = getString(lineTab);
			string storeName = getString(nameTab);
			displayName.text = storeName;
			//Convert the string StoreName into a Characters Enum type
			Characters parseCharacter = (Characters)System.Enum.Parse (typeof(Characters), storeName);
			manageCharacter.character = parseCharacter;
			//Tell the character manager to go ahead and display the character
			CharacterManager.playScene = true;

			CheckLine = PrintThisLine;
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
	string fetchTab (int whichTab) {
		string[] indexTabs = LineGetter(PrintThisLine).Split("\t"[0]);
		Debug.Log("Number of tabs: " + indexTabs.Length);

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
