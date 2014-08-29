using UnityEngine;
using System.Collections;
using System.Text.RegularExpressions;

public class InputData : MonoBehaviour {

	public string inputText; //This is the text being typed by you
	bool getText = false; //When True, allows you to input text
	int textStep = 0; //Determines where in the process of entering data you are

	public TextMesh thisIP; //The Text that indicates the IP address in the upper right
	public TextMesh thisPort; //Text that indicates the current Port in the upper right

	public TextMesh showInput; //This is what you are currently typing
	TextMesh thisInputText;  //Instance of showInput

	public TextMesh showInstruction; //Used to tell you what to do
	TextMesh thisInstruction; 

	public string sendIP; //sends IP info once confirmed
	public int sendPort; // sends port info once confirmed
	
	public bool loadStream = false; // true if this is a stream loading scene
	bool reloadStream = false; //used to return to the loading scene 
	public static bool resetStreamer = false; //tells streamer to stop


	void Start () {
		getText = false;
		thisInputText = showInput.gameObject.GetComponent<TextMesh>();
		thisInstruction = showInstruction.gameObject.GetComponent<TextMesh>();
		thisIP.text = Constants.IP1;
		thisPort.text = Constants.Port1.ToString();

		if (loadStream == true) {
			thisInstruction.text = "Press Enter";
		}
	}
	
	void Update () {

		if (getText == true) {
			enterText ();
			if (textStep == 0) {
				thisInstruction.text = "Enter IP";
			} else if (textStep == 1) {
				thisInstruction.text = "Enter Port:";
			} 
		} else if (loadStream == true) {
			thisInstruction.text = "Press Enter";
		} else if (reloadStream == true) {
			thisInstruction.text = "Reload Stream? y/n";
		} else {
			thisInstruction.text = "";
		}

		if (Input.GetKeyDown("return") && getText == false && loadStream == true) {

			if (textStep >= 0) {
				getText = true;
			} else {
				textStep = 0;
			} 
		}
		else if (Input.GetKeyDown("return") && loadStream == false) {

			reloadStream = true;
			thisInstruction.text = "Reload Stream? y/n";
		}

		if (reloadStream == true && Input.GetKeyDown("y")) {
			resetStreamer = true;
			Application.LoadLevel("loadStream");
		} else if (reloadStream == true && Input.GetKeyDown ("n")) {
			reloadStream = false;
			thisInstruction.text = "";
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
					Constants.IP1 = inputText;
					Debug.Log(Constants.IP1);
					textStep++;
					
				} else  {
					//dont ever put get component in the update loop, except for also here!
					thisPort.gameObject.GetComponent<TextMesh>().text = inputText;
					var checkInt = 0;
					int.TryParse(inputText, out checkInt);
					if (checkInt > 0 && checkInt < 10000) {
						sendPort = checkInt;
						Constants.Port1 = checkInt;
						Debug.Log (Constants.Port1);
						textStep = -1;
						getText = false;
						Application.LoadLevel("runStream");
					}
				}

				inputText = "";

			} else {
				inputText += c;
				//getText = false;
			}
		}
	} //end of enterText
	

}//end of class
