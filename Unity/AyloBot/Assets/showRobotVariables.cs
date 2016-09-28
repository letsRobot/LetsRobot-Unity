using UnityEngine;
using System.Collections;

public class showRobotVariables : MonoBehaviour {


	//Print the variables that apply to this robot, meant for debugging.
	public TextMesh rLogMesh;
	public Renderer showLogger;
	string rPrint = "";
	Robot robot;

	// Use this for initialization
	void Start () {

		rLogMesh = this.gameObject.GetComponent<TextMesh>();
		showLogger = this.gameObject.GetComponent<Renderer>();
	
	}
	
	// Update is called once per frame
	void Update () {

		rPrint += ("Robot's Name: ") + Constants.robotName + ("\n");
		rPrint += ("\n") + ("IMU Variables: \n");
		rPrint += ("X: ") + Constants.imuEuler.x + ("\n");
		rPrint += ("Y: ") + Constants.imuEuler.y + ("\n");
		rPrint += ("Z: ") + Constants.imuEuler.z + ("\n \n");
	
		rPrint += ("Pan Head: ") + Constants.headPan + ("\n");
		rPrint += ("Tilt Head: ") + Constants.headTilt + ("\n\n");

		rPrint += ("Pen X: ") + Constants.penX + ("\n");
		rPrint += ("Pen Y: ") + Constants.penY + ("\n");

		rLogMesh.text = rPrint;
		rPrint = "";

		if (Input.GetKeyDown(KeyCode.Alpha0) && Constants.showVariables == true) {
				Constants.showVariables = false;
		} else if (Input.GetKeyDown(KeyCode.Alpha0) && Constants.showVariables == false) { 
				Constants.showVariables = true;
		}

		if (Constants.showVariables == true) {
			showLogger.enabled = true;
		} else {
			showLogger.enabled = false;
		}
	}
}
