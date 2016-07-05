using UnityEngine;
using System.Collections;


public class roboSim : MonoBehaviour {

	public GameObject robotBody; //Body of the robot.
	public GameObject rightWheel;
	public GameObject leftWheel;

	public Material[] wheelStates;
	Renderer leftRend;
	Renderer rightRend;
	Rigidbody Body;

	bool moveLeftWheel = false;
	bool moveRightWheel = false;
	float moveTime = 1.0f;
	bool triggerMovement = false;

	public float turnSpeed = 0.2f; //How fast do i turn?
	public float moveSpeed = 0.2f; //How fast does it look like i move?

	// Use this for initialization
	void Start () {

		//Robot will be moved by acting upon a rigid body... I think : D
		Body = robotBody.GetComponent<Rigidbody> ();
		leftRend = leftWheel.GetComponent<Renderer> ();
		rightRend = rightWheel.GetComponent<Renderer> ();
	

	}
	
	// Update is called once per frame
	void Update () {


		var inputSignal = simulateInput ();


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
			//leftMat = wheelStates[1];
			leftRend.sharedMaterial = wheelStates[1];
			rightRend.sharedMaterial = wheelStates[1];

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
			leftRend.sharedMaterial = wheelStates [0];
			rightRend.sharedMaterial = wheelStates [0];
			Debug.Log ("Movement Complete");

			//yield return null;
		} else {

			Debug.Log("We already movin yo!");
		}
	}
}
