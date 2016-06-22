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

	// Use this for initialization
	void Start () {
		Debug.Log("Starting up skynet");
		//robotMessages = new RobotMessages("64.185.234.177", 40100);
		robotMessages = Constants.skyNetMessages;
		robotMessages.SendMessage("login letsrobot");
		//running = true;
		refreshDevices();

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

	// Update is called once per frame
	void Update () {
		//if (!running) return;
		//robotMessages.SetServer(IP1, 40100);
		variables = robotMessages.GetVariables();
		if (variables.ContainsKey("devices") && !gotDevices) {
			//Debug.Log("got devices");
			devices=new List<IDictionary<string, string>>();
			//Debug.Log(variables["devices"]);
			var deviceList=JSON.Parse(variables["devices"]);
			Debug.Log("There are "+deviceList.Count+" devices connected");
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
				//Debug.Log (device);
				devices.Add (device);
				//devicesList[i].internalIp
			}
			//Debug.Log (devices);
			gotDevices=true;

			// EXAMPLE, connect to first bot on the list
			Debug.Log("connecting to "+devices[0]["internalIp"]);
			SelectDevice(devices[0]["internalIp"]);
			// END EXAMPLE
		}
		// unity "Robot" object may or may not be created yet
		// after it's created we need access to it
		// so after creation it needs to hook with us
		if (runTelly) {
			if (GameObject.Find ("Robot")) {
				Robot robot = GameObject.Find("Robot").GetComponent<Robot>();
				Constants.roboStuff.Update(variables, robotMessages);
					foreach (var command in robotMessages.GetCommands()) {
						Constants.roboStuff.Command (command, variables, robotMessages); 
				}
					} else {
					Debug.Log ("Still looking for robot");
				}
			}
		}

	void SelectDevice(string IP) {
		Constants.IP1 = IP;
		robotMessages.SendMessage ("iam " + IP);
		runTelly = true;
		Application.LoadLevel ("runTelly");

	}

	void UIExample() {
		if (gotDevices) {
			for (int i=0; i<devices.Count; i++) {
				var ip = devices[i]["internalIp"];
			}
		}
	}

}

