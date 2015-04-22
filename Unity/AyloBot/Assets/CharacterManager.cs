using UnityEngine;
using System.Collections;

public enum Characters {
	OPERATOR,
	ROBAD,
}

public enum Emotes {
	HAPPY,
	WORRIED,
}

public class CharacterManager : MonoBehaviour {

	public Characters character; //Character Name
	public Emotes emote; //Character Emote
	public GameObject[] characterObjects; //All HUD characters
	CharacterID[] makeCharacter; //Stores all the characters with IDs attached to this object

	public static bool playScene = false; //True when executing Scene
	public static bool sceneActive = false;

	GameObject itsAlive; //Current Active character
	bool setTimer = false; //triggers the countdown to time out the current scene

	void updateCutScene() {

		if (playScene == true) {

			if (itsAlive != null) {
				itsAlive.SetActive(false);
			}
			findEmote(character);
			if (itsAlive != null) {
				//Debug.Log("I am activating this character: " + itsAlive.name);
				//Debug.Log("Character Enum sayz: " + character.ToString() + " " + emote.ToString());
				itsAlive.SetActive(true);
			}
			playScene = false;
			sceneActive = true;
			setTimer = true;
		} 

		if (sceneActive == false && itsAlive != null) {
			itsAlive.SetActive(false);
		}

		if (setTimer == true) {
			setTimer = false;
			if (runSceneTimer == true) {
				//Reset Scene Timer if it's already running.
				StopCoroutine("sceneTimer");
				runSceneTimer = false;
				ImportSpreadSheet.sendLine = false;
			}
			StartCoroutine("sceneTimer");
		}
	}

	// Use this for initialization
	void Start () {

		playScene = false;
		//creat array of Character ID's to match the number of attached character objects
		makeCharacter = new CharacterID[characterObjects.Length];
		for (int i = 0; i < characterObjects.Length; i++) {
			//Let's make sure all the character objects are disabled
			characterObjects[i].SetActive(false);
			//Let's also get the CharacterID of each character object
			makeCharacter[i] = characterObjects[i].GetComponent<CharacterID>();
			//Debug.Log ("Character ID's: " + makeCharacter.Length);

		}
	}

	//find the character with the associated emote
	void findEmote (Characters myCharacter) {

		//Debug.Log("Character Enum sayz (find emote): " + myCharacter.ToString() + " " + emote.ToString());
		for (int i = 0; i < characterObjects.Length; i++) {
			if (emote == makeCharacter[i].emote && myCharacter == makeCharacter[i].character) {
				itsAlive = characterObjects[i];
				//Debug.Log("Emote this character: " + itsAlive.name);
			} 
		} 
	}

	
	// Update is called once per frame
	void Update () {

		updateCutScene();

	}

	bool runSceneTimer = false; //Basically used to check if coroutine is running to prevent multiple instances of it.
	 IEnumerator sceneTimer () {
		runSceneTimer = true;
		yield return new WaitForSeconds (ImportSpreadSheet.sceneTime);
		runSceneTimer = false;
		sceneActive = false;
		ImportSpreadSheet.sendLine = false;
		Debug.Log("Send Line Complete");
	}
}
