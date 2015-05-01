using UnityEngine;
using System.Collections;

public class technicalDifficulties : MonoBehaviour {

	public GameObject td;
	public GameObject pleaseStandBy;
	//GameObject thisTD;

	// Use this for initialization
	void Start () {
		//thisTD = td.GameObject.GetComponent<GameObject>();
		td.SetActive(false);
		pleaseStandBy.SetActive(false);
	
	}
	
	// Update is called once per frame
	void Update () {

		if (Input.GetKeyDown(KeyCode.T) && Constants.TD == false) {
			td.SetActive(true);
			Constants.TD = true;
		} else if (Input.GetKeyDown(KeyCode.T) && Constants.TD == true) {
			td.SetActive(false);
			Constants.TD = false;
		}

		if (Input.GetKeyDown(KeyCode.S) && Constants.standBy == false) {
			pleaseStandBy.SetActive(true);
			Constants.standBy = true;
		} else if (Input.GetKeyDown(KeyCode.S) && Constants.standBy == true) {

			pleaseStandBy.SetActive(false);
			Constants.standBy = false;
		}
	}
}
