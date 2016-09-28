using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using SimpleJSON;

public class skynetConnect : MonoBehaviour {
	
	RobotMessages robotMessages;
	bool gotDevices=false;
	IDictionary<string, string> variables;
	IList<IDictionary<string, string>> devices;
	JSONNode deviceList;
	int countDevices = -1;

	bool runTelly=false;

	//connection log variables
	public TextMesh cLogPrint;
	string cLog = ("Loading ...");

	public TextMesh displayRobots;
	string printR= ("Robots"); //Printout the robot info on the loading screen

	public int checkRate = 1;
	bool checkConnection = false;


	// Use this for initialization
	void Start () {
		cLogPrint.text += "Loading...";
		robotMessages = Constants.skyNetMessages;
		robotMessages.SendMessage("login letsrobot");

		//robotMessages.SendMessage("parse on"); // default: skynet will parse
		//robotMessages.SendMessage("parse off"); // unity handles parsing and cmds to robot
		//robotMessages.SendMessage("robo on"); // default: skynet will send cmds to robots
		//robotMessages.SendMessage("robo off"); // only works when parse is on: SkyNet parses but lets Unity send cmds to robot

		//running = true;
		refreshDevices();
		cLogPrint.text = "Ready to look for robots \n";

	}

	// You can run this to refresh the variable from SkyNet
	void refreshDevices() {

		if (checkConnection == false) {

			gotDevices = false;
			robotMessages.SendMessage ("devices");
			checkConnection = true;
			StartCoroutine("checkSkynet");

		}
	}

	public void OnApplicationQuit()
	{
		//running = false;
		robotMessages.Stop();
	}

	// Update is called once per frame
	void Update () {


		cLog += "Loading ...\n";
		refreshDevices ();

		checkForRobots ();

		//Load the robot object (variables related to the selected robot).
		if (runTelly) {
			if (GameObject.Find ("Robot")) {
				Constants.roboStuff.Update(variables, robotMessages);
					foreach (var command in robotMessages.GetCommands()) {
						Constants.roboStuff.Command (command, variables, robotMessages); 
				}
					} else {
					cLog += ("Loading Robot HUD");
				}
			}

		cLogPrint.text = cLog;
		Debug.Log(cLog);
		cLog = ("");

		displayRobots.text = printR;


		}
	

	//Picks the robot we are using for this session.
	void SelectDevice(string IP) {
		Constants.IP1 = IP; //This is the IP that is being selected to run.
		robotMessages.SendMessage ("iam " + IP);
		runTelly = true;
		Constants.robotLive = true; //Send status that the robot is now live.
		Application.LoadLevel ("runTelly");

	}

	void UIExample() {
		if (gotDevices) {
			for (int i=0; i<devices.Count; i++) {
				var ip = devices[i]["internalIp"];
				Debug.Log (ip[i]);
			}
		}
	}

	void checkForRobots() {
		
		variables = robotMessages.GetVariables();
		if (variables.ContainsKey("devices") && !gotDevices) {
			//Debug.Log("got devices");
			devices=new List<IDictionary<string, string>>();
			//Debug.Log(variables["devices"]);
			deviceList=JSON.Parse(variables["devices"]);
			if (deviceList.Count != countDevices) {
				cLog += ("There are ") + deviceList.Count+ (" devices connected \n");
				countDevices = deviceList.Count;
			}
			
			if (deviceList.Count != 0) {
				printR = ("");
				for(int i=0; i<deviceList.Count; i++) {
					//Debug.Log("got device");
					//Debug.Log (deviceList[i]);
					//new Dictionary<string, string>(variables);
					var device=new Dictionary<string, string>();
					device["internalIp"]=deviceList[i]["internalIp"];
					device["botVersion"]=deviceList[i]["botVersion"];
					device["last"]=deviceList[i]["last"];
					device["mac"]=deviceList[i]["mac"];
					device["shortName"]=deviceList[i]["shortName"];
					Debug.Log (device);
					devices.Add (device);
					//devicesList[i].internalIp
					
					gotDevices=true;
					printR += "Robots Found... Getting Info \n";
					printR += devices[i]["shortName"] + ("\n");
					printR += ("IP: ") + devices[i]["internalIp"] + ("\n \n");
				}
				
			} else {
				cLog += "No robots found, still looking... \n";
				printR = "No robots found"; 
			}
		}
		robotSelector ();
	}


	//Logic for selecting Robots to run
	int pickRobot = 0;
	void robotSelector () {
		//This could probably use some better selection logic at some point.
		if (Input.GetKey (KeyCode.Alpha1) && devices[0] != null) {

			cLog += ("Picking Robot # ") + pickRobot;
			pickRobot = 1;
			Debug.Log ("Thing is pressed yo");
			SelectDevice(devices[0]["internalIp"]);
			Constants.robotName = devices[0]["shortName"];

		} else if (Input.GetKey (KeyCode.Alpha2) && devices[1] != null) {
			pickRobot = 2;
			cLog += ("Picking Robot # ") + pickRobot;
			SelectDevice(devices[1]["internalIp"]);
			Constants.robotName = devices[1]["shortName"];

		} else if (Input.GetKey (KeyCode.Alpha3) && devices[2] != null) {
			pickRobot = 3;
			cLog += ("Picking Robot # ") + pickRobot;
			SelectDevice(devices[2]["internalIp"]);
			Constants.robotName = devices[2]["shortName"];
			
		}else if (Input.GetKey (KeyCode.Alpha4) && devices[3] != null) {
			pickRobot = 4;
			cLog += ("Picking Robot # ") + pickRobot;
			SelectDevice(devices[3]["internalIp"]);
			Constants.robotName = devices[3]["shortName"];
			
		}
	}

	IEnumerator checkSkynet () {

			yield return new WaitForSeconds (checkRate);
			checkConnection = false;

	}
}

