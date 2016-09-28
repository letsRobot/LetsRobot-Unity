using UnityEngine;
using System.Collections;

public class showRobotVariables : MonoBehaviour {


	//Print the variables that apply to this robot, meant for debugging.
	public TextMesh rLogMesh;
	string rPrint = "";
	Robot robot;

	// Use this for initialization
	void Start () {

		rLogMesh = this.gameObject.GetComponent<TextMesh>();
	
	}
	
	// Update is called once per frame
	void Update () {

		rPrint += ("Robot's Name: ") + Constants.robotName + ("\n");
		rPrint += ("\n") + ("IMU Variables: \n");
		rPrint += ("X: ") + Constants.imuEuler.x + ("\n");
		rPrint += ("Y: ") + Constants.imuEuler.y + ("\n");
		rPrint += ("Z: ") + Constants.imuEuler.z + ("\n \n");
	
		rPrint += ("Pan Head: ") + Constants.headPan + ("\n");
		rPrint += ("Tilt Head: ") + Constants.headTilt + ("\n");

		rLogMesh.text = rPrint;
		rPrint = "";
	
	}
}
