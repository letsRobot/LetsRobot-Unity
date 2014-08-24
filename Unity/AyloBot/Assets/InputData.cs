using UnityEngine;
using System.Collections;

public class InputData : MonoBehaviour {

	public string inputText;
	bool getText = false;
	int textStep = 0;

	public TextMesh thisIP;
	public TextMesh thisPort;

	public TextMesh showInput; //This is what you are currently typing
	TextMesh thisInputText;  //Instance of showInput

	public TextMesh showInstruction;
	TextMesh thisInstruction;

	public GameObject streamer;

	void Start () {
		getText = false;
		thisInputText = showInput.gameObject.GetComponent<TextMesh>();
		thisInstruction = showInstruction.gameObject.GetComponent<TextMesh>();
	}
	
	void Update () {

		if (getText == true) {
			enterText ();
			if (textStep == 0) {
				thisInstruction.text = "Enter IP";
			} else if (textStep == 1) {
				thisInstruction.text = "Enter Port:";
			} 
		} else {
			thisInstruction.text = "";
		}

		if (Input.GetKeyDown("return") && getText == false) {

			if (textStep >= 0) {
				getText = true;
			} else {
				textStep = 0;
			}
		}
		thisInputText.text = inputText;
	} //end of Update

	void enterText () {

		foreach (char c in Input.inputString) {
			if (c == "\b"[0]) {

				if (inputText.Length != 0) {
					inputText = inputText.Substring(0, inputText.Length - 1);
			}
		} else if (c == "\n"[0] || c == "\r"[0]) {
					//print("User has entered IP: " + inputText);

				if (textStep == 0) {
					//dont ever put get component in the update loop, except for here!
					thisIP.gameObject.GetComponent<TextMesh>().text = inputText;
					//streamer.gameObject.GetComponent<Streamer>().address = inputText;
					textStep++;
					
				} else  {
					//dont ever put get component in the update loop, except for also here!
					thisPort.gameObject.GetComponent<TextMesh>().text = inputText;
					//streamer.gameObject.GetComponent<Streamer>().port = inputText;
					//int.TryParse(inputText, out streamer.gameObject.GetComponent<Streamer>().port);
					//int portInt = 0;
					//int.TryParse(inputText, out portInt)
					//if (portInt > 0 && portInt < 10000) {
						//streamer.gameObject.GetComponent<Streamer>().port = portInt;
					//}
					textStep = -1;
					getText = false;
				}

				inputText = "";

			} else {
				inputText += c;
				//getText = false;
			}
		}
	} //end of enterText


}
