using UnityEngine;
using System.Collections;

public class technicalDifficulties : MonoBehaviour {

	public GameObject td;
	public GameObject pleaseStandBy;
	public GameObject gameOver;
	//GameObject thisTD;
	//public bool triggerScreen;

	// Use this for initialization
	void Start () {
		//triggerScreen = false;
		td.SetActive(false);
		pleaseStandBy.SetActive(false);
		gameOver.SetActive (false);
	
	}

	//This code is used to toggle screens like Game Over and Stand By
	//You can change this status by using the following key strokes
	void checkKeys () {
		if (Input.GetKeyDown (KeyCode.T)) {
			Constants.updateTD = true;
		}

		if (Input.GetKeyDown (KeyCode.S)) {
			Constants.updateStandBy = true;
		}

		if (Input.GetKeyDown (KeyCode.G)) {
			Constants.updateGameOver = true;
		}
	}
	
	// Update is called once per frame
	void Update () {

		checkKeys ();

		//Changes to the screen can also be accessed via robotStuff.cs, which is talking directly to the robot.

		if (Constants.updateTD == true && Constants.TD == false) {
			td.SetActive (true);
			Constants.TD = true;
			Constants.updateTD = false;
		} else if (Constants.updateTD == true && Constants.TD == true) {
			td.SetActive (false);
			Constants.TD = false;
			Constants.updateTD = false;
		}

		if (Constants.updateStandBy == true && Constants.standBy == false) {
			pleaseStandBy.SetActive (true);
			Constants.standBy = true;
			Constants.updateStandBy = false;
		} else if (Constants.updateStandBy == true && Constants.standBy == true) {
			pleaseStandBy.SetActive (false);
			Constants.standBy = false;
			Constants.updateStandBy = false;
		}

		if (Constants.updateGameOver == true && Constants.gameOver == false) {
			gameOver.SetActive (true);
			Constants.gameOver = true;
			Constants.updateGameOver = false;
		} else if (Constants.updateGameOver == true && Constants.gameOver == true) {
			gameOver.SetActive (false);
			Constants.gameOver = false;
			Constants.updateGameOver = false;
		}
	}
}
