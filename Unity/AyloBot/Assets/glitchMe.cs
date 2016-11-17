using UnityEngine;
using System.Collections;
using Kino;

public class glitchMe : MonoBehaviour {

	public GameObject thisCamera;
	private AnalogGlitch editAG;
	private DigitalGlitch editDG;



	// Use this for initialization
	void Start () {
	
		editAG = thisCamera.GetComponent<AnalogGlitch>();
		editDG = thisCamera.GetComponent<DigitalGlitch>();
	}
	
	// Update is called once per frame
	void Update () {



		if (Constants.glitching == false && Constants.triggerGlitch == true) {
			editAG.enabled = true;
			editDG.enabled = true;
			Constants.triggerGlitch = false;
			Constants.glitching = true;
		} else if (Constants.glitching == true && Constants.triggerGlitch == true) {
			editAG.enabled = false;
			editDG.enabled = false;
			Constants.triggerGlitch = false;
			Constants.glitching = false;
		}
	}
}