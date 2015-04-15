using UnityEngine;
using System.Collections;

public class levitateMe : MonoBehaviour {
	
	Vector3 origin; //starting position of object
	Vector3 target; //disired position
	public float levRange= 1.0f; //the distance the objeject will move as it levitates
	public float levSpeed = 1.0f; //the speed at which the object moves between its origin and target positions
	bool nextHover; //triggers movement up or down

	bool startHovering;
	//public bool offsetOrigin;

	void Start () {

		//initialized variables at start...
		origin = transform.localPosition;
		//print(origin.y);
		target = new Vector3(origin.x, origin.y + levRange, origin.z);

		//change starting direction
//		if (offsetOrigin == true) {
//			Vector3 switchPositions = target;
//			target = origin;
//			origin = switchPositions;
//			this.gameObject.transform.position = origin;
//		}
		nextHover = true;
		startHovering = false;
		StartCoroutine("randomStartTime");
	
		}

	void Update () {

		if (startHovering == true) {

		//Makes object move up
		if (Vector3.Distance(transform.localPosition, origin) < 0.05f || nextHover == true) {
			transform.localPosition = Vector3.Slerp(transform.localPosition, target, levSpeed * Time.deltaTime);
			nextHover = true;
		} 
		//makes object move down
		if (Vector3.Distance(transform.localPosition, target) < 0.05f || nextHover == false) {
		transform.localPosition = Vector3.Slerp(transform.localPosition, origin, levSpeed * Time.deltaTime);
			nextHover = false;
			
			}
		}
	}
	
	//This method doesn't seem to work.
	IEnumerator levitating () {


		Debug.Log("starting levitation");
		transform.localPosition = Vector3.Lerp(transform.localPosition, target, levSpeed * Time.deltaTime);
		yield return null;

		Debug.Log("Levitation Next Step");
		transform.localPosition = Vector3.Lerp(transform.localPosition, origin, levSpeed *Time.deltaTime); 
		yield return null;

		nextHover = true;

	
	}

	IEnumerator randomStartTime () {

		yield return new WaitForSeconds(Random.Range(0.0f, levSpeed));
		startHovering = true;

	}
}
