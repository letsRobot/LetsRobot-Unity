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

	public Characters character;
	public Emotes emote;
	public GameObject[] characterObjects;
	CharacterID[] makeCharacter;
	int characterIndex;

	public bool playScript;

	GameObject itsAlive;

	// Use this for initialization
	void Start () {

		playScript = false;
		makeCharacter = new CharacterID[characterObjects.Length];
		for (int i = 0; i < characterObjects.Length; i++) {
			characterObjects[i].SetActive(false);
			makeCharacter[i] = characterObjects[i].GetComponent<CharacterID>();
			Debug.Log ("Character ID's: " + makeCharacter.Length);

		}
		findEmote(character);
		if (itsAlive != null) {
			itsAlive.SetActive(true);

		}
	}

	void findEmote (Characters myCharacter) {

		for (int i = 0; i < characterObjects.Length; i++) {
			if (emote == makeCharacter[i].emote) {
				itsAlive = characterObjects[i];
				Debug.Log("Emote this character: " + i);
			} 
		} 

	}


	
	// Update is called once per frame
	void Update () {


	
	}
}
