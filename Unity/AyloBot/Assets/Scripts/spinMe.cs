using UnityEngine;
using System.Collections;

public class spinMe : MonoBehaviour {

	//Rotate objects along the following axis
	public float x;
	public float y;
	public float z;

	//set speed for rotation
	public float rotationSpeed;

	public bool rotateToZero;

	// Update is called once per frame
	void Update () {

		if (rotateToZero == true) {

			Quaternion myRot = Quaternion.Euler(x,y,z);
			transform.rotation =  Quaternion.Slerp(transform.rotation, myRot, rotationSpeed * Time.deltaTime);

		} else {

		//You spin me right round baby right round like a round thing right round right round...
		transform.Rotate (x,y,z * rotationSpeed * Time.deltaTime);

		}
	}
}
