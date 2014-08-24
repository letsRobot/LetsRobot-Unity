using UnityEngine;
using System.Collections;
using System.Text.RegularExpressions;

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

	public string sendIP;
	public int sendPort;

	public GameObject Streamer;
	private GameObject myStreamer;


	void Start () {
		getText = false;
		thisInputText = showInput.gameObject.GetComponent<TextMesh>();
		thisInstruction = showInstruction.gameObject.GetComponent<TextMesh>();
		myStreamer = Streamer.gameObject.GetComponent<Transform>().gameObject;

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
					sendIP = inputText;
					textStep++;
					
				} else  {
					//dont ever put get component in the update loop, except for also here!
					thisPort.gameObject.GetComponent<TextMesh>().text = inputText;
					var checkInt = 0;
					int.TryParse(inputText, out checkInt);
					if (checkInt > 0 && checkInt < 10000) {
						sendPort = checkInt;
						textStep = -1;
						getText = false;
						//resetStream();
					}
				}

				inputText = "";

			} else {
				inputText += c;
				//getText = false;
			}
		}
	} //end of enterText

	void resetStream () {

		Destroy(myStreamer.gameObject);
		myStreamer = (GameObject) Instantiate(Streamer);
		Streamer newStreamer = myStreamer.gameObject.GetComponent<Streamer>();
		newStreamer.address = sendIP;
		newStreamer.port = sendPort;

	} //end of resetStream

}//end of class
