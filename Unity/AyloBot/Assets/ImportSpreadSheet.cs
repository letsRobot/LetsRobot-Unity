using UnityEngine;
using System.Collections;
using System.IO;

public enum ImportTabs {

	TAG,
	CHARACTER,
	EMOTE,
	TEXT,
	TIME,
	SFX,

}

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

	bool wrongLine;


	void initializeStory () {

		//References to HUD objects
		displayName = myName.gameObject.GetComponent<TextMesh>();
		displayText = myText.gameObject.GetComponent<TextMesh>();

	}

	string characterTalks() {

		string character = fetchTab(1);
		string emote = fetchTab(2);
		string talk = fetchTab(3);
		return talk;

	}

	//Gets the desired tab from any particular line in StoryData
	string fetchTab (int whichTab) {
		string[] indexTabs = LineGetter(PrintThisLine).Split("\t"[0]);
		Debug.Log("Number of tabs: " + indexTabs.Length);

		if (wrongLine == true) {
			string oops = "Zomg Internal error! We are going dowin in fames!";
			return oops;
		} else {
			return indexTabs[whichTab];
		}

	}

	// Use this for initialization
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

		if (CheckLine != PrintThisLine) {
			displayText.text = characterTalks();

			CheckLine = PrintThisLine;
		}
	}

	int FetchLine = 1; //Start at 1 so we can pretend line 0 doesn't exist!

	//Indexes and fetches the requested line from the StoryData Sheet
	string LineGetter (int WhichLine) {
		string[] indexLines = ReadSheet.Split("\n"[0]);
		FetchLine = indexLines.Length;

		for (int i = 0; i < FetchLine; i++) {
			string PrintStrings = indexLines[i];
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
