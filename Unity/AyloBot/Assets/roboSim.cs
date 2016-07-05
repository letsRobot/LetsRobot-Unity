﻿using UnityEngine;
using System.Collections;


public class roboSim : MonoBehaviour {

	//Robot Body & General Movement
	public GameObject robotBody; //Body of the robot.
	Rigidbody Body;
	public float turnSpeed = 0.2f; //How fast do i turn?
	public float moveSpeed = 0.2f; //How fast does it look like i move?

	//Global Management
	public Material[] statusMaterials; //blue, green, yellow, red

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
	
	Vector3 leftGripperPos;
	Vector3 leftGripClosePos;
	Vector3 rightGripperPos;
	Vector3 rightGripClosePos;
	Vector3 leftGripTargetPos;
	Vector3 rightGripTargetPos;

	public bool closeGripper = false;
	bool triggerGrip = false;
	public float gripperMoveDist = 1.0f;
	public float gripSpeed = 1.0f;

	Renderer leftGripperRend;
	Renderer rightGripperRend;




	// Use this for initialization
	void Start () {

		//Robot will be moved by acting upon a rigid body... I think : D
		Body = robotBody.GetComponent<Rigidbody> ();

		//Get Renderer for Left and Right Wheels
		leftRend = leftWheel.GetComponent<Renderer> ();
		rightRend = rightWheel.GetComponent<Renderer> ();


		//Managing gripper Setup
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
	

	}

	void moveGripper() {

		//Debug.Log ("Left Gripper Pos: " + leftGripperPos);
		//Debug.Log ("Right Gripper Pos: " + rightGripperPos);
		//Debug.Log ("Gripper Targets: " + leftGripTargetPos + " " + rightGripTargetPos);

		if (Input.GetKeyDown (KeyCode.G)) {

			if (closeGripper == false) {
				closeGripper = true;
				leftGripTargetPos = leftGripClosePos;
				rightGripTargetPos = rightGripClosePos;
				//Debug.Log("Gripper Triggered");
			} else {
				leftGripTargetPos = leftGripperPos;
				rightGripTargetPos = rightGripperPos;
				closeGripper = false;
			}
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

		//if (Vector3.Distance(leftGripCur, origin) < 0.05f 
		
	}

	
	// Update is called once per frame
	void Update () {


		var inputSignal = simulateInput ();
		moveGripper ();


		if (inputSignal.sqrMagnitude > 0.05f * 0.05f) {
			//Body.MovePosition(Body.position + (moveSpeed * Time.deltaTime) * inputSignal); //move in direction
			var targetRotation = Quaternion.LookRotation(inputSignal);
			Body.MoveRotation(Body.rotation.EaseTowards(targetRotation, turnSpeed));
		}
		
	}

	Vector3 simulateInput() {

		var simulateInput = new Vector3 (
				Input.GetAxisRaw("Horizontal"),
				Input.GetAxisRaw("Zed"),
				Input.GetAxisRaw("Vertical")
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
}