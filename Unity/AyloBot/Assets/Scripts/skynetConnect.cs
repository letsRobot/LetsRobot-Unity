using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using SimpleJSON;

public class skynetConnect : MonoBehaviour {
	
	RobotMessages robotMessages;
	bool gotDevices=false;
	//bool running=false;
	IDictionary<string, string> variables;
	IList<IDictionary<string, string>> devices;
	bool runTelly=false;
	public TextMesh rlog;
	JSONNode deviceList;
	int countDevices = -1;

	// Use this for initialization
	void Start () {
		rlog.text += "Loading...";
		robotMessages = Constants.skyNetMessages;
		robotMessages.SendMessage("login letsrobot");

		//robotMessages.SendMessage("parse on"); // default: skynet will parse
		//robotMessages.SendMessage("parse off"); // unity handles parsing and cmds to robot
		//robotMessages.SendMessage("robo on"); // default: skynet will send cmds to robots
		//robotMessages.SendMessage("robo off"); // only works when parse is on: SkyNet parses but lets Unity send cmds to robot

		//running = true;
		refreshDevices();
		rlog.text = "Ready to look for robots \n";

	}

	// You can run this to refresh the variable from SkyNet
	void refreshDevices() {
		gotDevices = false;
		robotMessages.SendMessage("devices");
	}

	public void OnApplicationQuit()
	{
		//running = false;
		robotMessages.Stop();
	}

	void checkForRobots() {

	}

	// Update is called once per frame
	void Update () {

		variables = robotMessages.GetVariables();
		if (variables.ContainsKey("devices") && !gotDevices) {
			//Debug.Log("got devices");
			devices=new List<IDictionary<string, string>>();
			//Debug.Log(variables["devices"]);
			deviceList=JSON.Parse(variables["devices"]);
			if (deviceList.Count != countDevices) {
				rlog.text += ("There are ") + deviceList.Count+ (" devices connected \n");
				countDevices = deviceList.Count;
			}

			if (deviceList.Count != 0) {
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
					rlog.text += "Robots Found... Getting Info \n";
					rlog.text += devices[i]["shortName"] + ("\n");
					rlog.text += ("IP: ") + devices[0]["internalIp"] + ("\n \n");
				}

			} else {
				rlog.text += "No robots found, still looking... \n";
			}

			if (deviceList.Count >= 1) {
				//Right now, this just goes with the first robot in the list.
				rlog.text += ("connecting to ") + devices[0]["internalIp"] + ("\n");
				SelectDevice(devices[0]["internalIp"]);
			}
			// END EXAMPLE
		}
		// unity "Robot" object may or may not be created yet
		// after it's created we need access to it
		// so after creation it needs to hook with us
		if (runTelly) {
			if (GameObject.Find ("Robot")) {
				//Robot robot = GameObject.Find("Robot").GetComponent<Robot>();
				Constants.roboStuff.Update(variables, robotMessages);
					foreach (var command in robotMessages.GetCommands()) {
						Constants.roboStuff.Command (command, variables, robotMessages); 
				}
					} else {
					Debug.Log ("Still looking for robot");
				}
			}
		}

	//Picks the robot we are using for this session.
	void SelectDevice(string IP) {
		Constants.IP1 = IP;
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

}

