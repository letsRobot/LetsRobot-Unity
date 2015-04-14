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

	public bool playScript;

	// Use this for initialization
	void Start () {

		playScript = false;
		makeCharacter = new CharacterID[characterObjects.Length];
		for (int i = 0; i < characterObjects.Length; i++) {
			characterObjects[i].SetActive(false);
			makeCharacter[i] = characterObjects[i].GetComponent<CharacterID>();
			Debug.Log ("Character ID's: " + makeCharacter.Length);
		}
	}

	void whoAmI () {

		switch (character) {
		case Characters.OPERATOR:
			Debug.Log("I am the Operator, nice to meet you!");
//			foreach (GameObject characterObject in characterObjects) {
//				if (characterObject.GetComponent<CharacterID>().character == Characters.OPERATOR)
//			}
			break;
		case Characters.ROBAD:
			Debug.Log("I am Robad, and i hate you");
			break;
		default:
			Debug.Log ("I am nobody, and i shouldn't be here");
			break;
		}

		switch (emote) {
		case Emotes.HAPPY:
			Debug.Log("I am so happy!");
			break;
		case Emotes.WORRIED:
			Debug.Log("I am a little worried");
			break;
		default:
			Debug.Log("I feel nothing, do i even exist?");
			break;
		}
	}
	
	// Update is called once per frame
	void Update () {


	
	}
}
