using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class roboSim : MonoBehaviour {

	//Robot Body & General Movement
	public GameObject robotBody; //Body of the robot.
	Rigidbody Body;
	public float turnSpeed = 0.2f; //How fast do i turn?
	public float moveSpeed = 0.2f; //How fast does it look like i move?

	float[] float4Imu = {0.0f, 0.0f, 0.0f, 0.0f};
	float[] float3Imu = {0.0f, 0.0f, 0.0f};

	//Body orientaiton in meat space
	Quaternion bodyRot;
	Quaternion getRot;

	//Global Management
	public Material[] statusMaterials; //blue, green, yellow, red
	Robot robot; //Reference to data from Robot Class
	IDictionary<string, string> IMUData;
	
	//Wheel Management
	public GameObject rightWheel;
	public GameObject leftWheel;

	//Renderer for the wheels
	Renderer leftRend;
	Renderer rightRend;

	bool moveLeftWheel = false;
	bool moveRightWheel = false;
	float moveTime = 1.0f;
	bool triggerMovement = false;
	

	//Gripper Management
	public GameObject gripperLeft;
	public GameObject gripperRight;

	/* Each gripper bit has an open and closed position stored, 
	   and a target position which switches between the two. */
	Vector3 leftGripperPos;
	Vector3 leftGripClosePos;
	Vector3 rightGripperPos;
	Vector3 rightGripClosePos;
	Vector3 leftGripTargetPos;
	Vector3 rightGripTargetPos;

	public bool closeGrip;
	bool triggerGrip = false;
	public float gripperMoveDist = 1.0f;
	public float gripSpeed = 1.0f;

	Renderer leftGripperRend;
	Renderer rightGripperRend;
	
	// Use this for initialization
	void Start () {

		//Find the robot object in order to get data to run the simulation
		if (GameObject.Find ("Robot")) {
			robot = GameObject.Find("Robot").GetComponent<Robot>();
		} else {
			//Debug.Log ("RoboSim not connecting to Robot");
		}



		//Robot will be moved by acting upon a rigid body... I think : D
		Body = robotBody.GetComponent<Rigidbody> ();
		bodyRot = robotBody.GetComponent<Transform> ().rotation;

		//Get Renderer for Left and Right Wheels
		leftRend = leftWheel.GetComponent<Renderer> ();
		rightRend = rightWheel.GetComponent<Renderer> ();


		//Managing gripper Setup

		//Get the gripper status from Constants, which is determined by the chat.
		closeGrip = Constants.gripperClose;
		leftGripperPos = gripperLeft.gameObject.GetComponent<Transform> ().localPosition;
		rightGripperPos = gripperRight.gameObject.GetComponent<Transform> ().localPosition;

		leftGripClosePos = new Vector3 (leftGripperPos.x + gripperMoveDist, 
		                                 leftGripperPos.y, 
		                                 leftGripperPos.z);

		rightGripClosePos = new Vector3 (rightGripperPos.x - gripperMoveDist,
		                                  rightGripperPos.y,
		                                  leftGripperPos.z);

		leftGripperRend = gripperLeft.GetComponent<Renderer> ();
		rightGripperRend = gripperRight.GetComponent<Renderer> ();

		rightGripTargetPos = rightGripperPos;
		leftGripTargetPos = leftGripperPos;
	

	}


	
	// Update is called once per frame
	void Update () {

			fetchIMU ();


		moveGripper ();

		if (Constants.robotLive == false) {
			var inputSignal = simulateInput ();
			if (inputSignal.sqrMagnitude > 0.05f * 0.05f) {
				//Body.MovePosition(Body.position + (moveSpeed * Time.deltaTime) * inputSignal); //move in direction
				var targetRotation = Quaternion.LookRotation (inputSignal);
				Body.MoveRotation (Body.rotation.EaseTowards (targetRotation, turnSpeed));
			}
		} else {

			bodyRot = Quaternion.Euler(0.0f, float3Imu[0], 0.0f);
			Body.MoveRotation (Body.rotation.EaseTowards (bodyRot, turnSpeed));
			//robotBody.transform.localRotation = bodyRot;


		}
	}

	void moveGripper() {

		closeGrip = Constants.gripperClose;

		//Debug.Log ("Left Gripper Pos: " + leftGripperPos);
		//Debug.Log ("Right Gripper Pos: " + rightGripperPos);
		//Debug.Log ("Gripper Targets: " + leftGripTargetPos + " " + rightGripTargetPos);

			
			if (closeGrip == true) {
				leftGripTargetPos = leftGripClosePos;
				rightGripTargetPos = rightGripClosePos;
				//Debug.Log("Gripper Triggered");
			} else if (closeGrip == false) {
				leftGripTargetPos = leftGripperPos;
				rightGripTargetPos = rightGripperPos;
			}
		
		gripperLeft.transform.localPosition = 
			Vector3.Lerp (gripperLeft.transform.localPosition, 
			              leftGripTargetPos, 
			              gripSpeed * Time.deltaTime);
		gripperRight.transform.localPosition = 
			Vector3.Lerp (gripperRight.transform.localPosition, 
			              rightGripTargetPos, 
			              gripSpeed * Time.deltaTime);
		
		
		var leftGripCur = gripperLeft.transform.localPosition;
		var rightGripCur = gripperRight.transform.localPosition;
		
		if (Vector3.Distance (leftGripCur, leftGripTargetPos) > 0.05f || 
			Vector3.Distance (rightGripCur, rightGripTargetPos) > 0.05f) {
			leftGripperRend.sharedMaterial = statusMaterials [1];
			rightGripperRend.sharedMaterial = statusMaterials [1];
		} else {
			leftGripperRend.sharedMaterial = statusMaterials[0];
			rightGripperRend.sharedMaterial = statusMaterials[0];
		}
	}


	Vector3 simulateInput() {

		var simulateInput = new Vector3 (
				Input.GetAxisRaw("Horizontal"),
				Input.GetAxisRaw("Vertical"),
				Input.GetAxisRaw("Zed")
				);

		if (Input.GetKeyDown (KeyCode.F)) {

			Debug.Log("Key F is Down Yo");
			moveLeftWheel = true;
			moveRightWheel = true;
			leftRend.sharedMaterial = statusMaterials[1];
			rightRend.sharedMaterial = statusMaterials[1];

			triggerMovement = true;
			StartCoroutine("moveDuration");

		}
		return simulateInput;
	}

	IEnumerator moveDuration () {

		if (triggerMovement == true && moveLeftWheel == true || moveRightWheel == true) {
			Debug.Log ("Movement Triggered");
			triggerMovement = false;
			yield return new WaitForSeconds (moveTime);
			moveLeftWheel = false;
			moveRightWheel = false;
			leftRend.sharedMaterial = statusMaterials [0];
			rightRend.sharedMaterial = statusMaterials [0];
			Debug.Log ("Movement Complete");

			//yield return null;
		} else {

			Debug.Log("We already movin yo!");
		}
	}

	void fetchIMU () {
		IMUData = robot.getIMUVariables();
		if (IMUData != null) {
			try {
			float qx = (float)Convert.ToDouble(IMUData["quaternion_x"]);
			float qy = (float)Convert.ToDouble(IMUData["quaternion_y"]);
			float qz = (float)Convert.ToDouble(IMUData["quaternion_z"]);
			float qw = (float)Convert.ToDouble(IMUData["quaternion_w"]);

			float ex = (float)Convert.ToDouble(IMUData["euler_heading"]);
			float ey = (float)Convert.ToDouble (IMUData["euler_roll"]);
			float ez = (float)Convert.ToDouble (IMUData["euler_pitch"]);

			

			float[] efloat = float3Imu;

			if (ex >= 0.0f) {
				efloat[0] = ex;
			}

			if (ey >= 0.0f) {
				efloat[1] = ey;
			}

			if (ez >= 0.0f) {
				efloat[2] = ez;
			}
			
			/*Debug.Log ("Euler Rotation - Heading:" + efloat[0] + 
			           " Roll: " + efloat[1] + 
			           " Pitch: " + efloat[2]);*/

			float3Imu = efloat;

			if (qx >= 0.0f) {
				float4Imu[0] = qx;
			}

			if (qy >= 0.0f) {
				float4Imu[1] = qy;
			}

			if (qz >= 0.0f) {
				float4Imu[2] = qz;
			}

			if (qw >= 0.0f) {
				float4Imu[3] = qw;
			}

			//Quaternion IMURot = new Quaternion(qx, qy, qz, qw);
			//bodyRot = IMURot;


			 getRot = new Quaternion(float4Imu[0],
			                         float4Imu[1],
			                         float4Imu[2],
			                         float4Imu[3]); 
			} catch(KeyNotFoundException) {}
			//getRot = new Quaternion(qx, qy, qz, qw);
		

			/*Debug.Log ("IMU = x: " + float4Imu[0] + 
			           " y: " + float4Imu[1] +
			           " z: " + float4Imu[2] +
			           " w: " + float4Imu[3]);*/

			//foreach (KeyValuePair<string, string> entry in IMUData) {
				//Debug.Log ("Key");
				//Debug.Log (entry.Key);
				//Debug.Log ("Value");
				//Debug.Log (entry.Value);
				}
			//} else {
				//Debug.Log("No IMU Data found");
		//}
	}
}