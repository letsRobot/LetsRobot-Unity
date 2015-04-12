using UnityEngine;
using System.Collections;

public class Equipment : MonoBehaviour {

	public GameObject mySword;
	public bool equipSword;
	public static bool robotEquipSword;
	bool equipped;

	public AudioClip equipSwordSound;
	AudioSource equipSFX;

	// Use this for initialization
	void Start () {

		//true if something is equipped
		equipped = false;

		//Sword Variables
		equipSword = false;
		robotEquipSword = false;
		robotSwordAttack = false;
		swordCanAttack = true;

		equipSFX = this.gameObject.GetComponent<AudioSource>();
		spinTheSword = mySword.gameObject.GetComponent<spinMe>();
		swordRotation = mySword.gameObject.GetComponent<Transform>().transform.rotation;
		swordPoiseRot = Quaternion.Euler(0, 0, 320);

	
	}
	
	// Update is called once per frame
	void Update () {

		equipSword = robotEquipSword;
		swordAttack = robotSwordAttack;

		if (equipSword == true && equipped == false) {
			mySword.SetActive(true);
			equipped = true;
			equipSFX.PlayOneShot(equipSwordSound);
		} else if (equipSword == false && equipped == true) {
			mySword.SetActive(false);
			Debug.Log("Ditch the sword yo!");
			equipped = false;
			equipSFX.PlayOneShot(equipSwordSound);
		}
	
		if (equipSword == true && equipped == true) {
			swordSwing();
		}

	}

	//ATTACK WITH THE SWORD
	spinMe spinTheSword;
	Quaternion swordRotation;
	Quaternion swordPoiseRot;
	public bool swordAttack = false;
	bool swordCanAttack = true;
	public static bool robotSwordAttack = false;
	public float swordAttackTime;
	public AudioClip swordAttackSFX;

	void swordSwing () {

		if (robotSwordAttack == true && swordCanAttack == true) {
			mySword.transform.rotation = swordPoiseRot;
			equipSFX.PlayOneShot(swordAttackSFX);
			spinTheSword.enabled = true;
			swordCanAttack = false;
			StartCoroutine("swordAttackTimer");
		}
	}

	IEnumerator swordAttackTimer () {
		yield return new WaitForSeconds (swordAttackTime);
		spinTheSword.enabled = false;
		//swordAttack = false;
		swordCanAttack = true;
		robotSwordAttack = false;
		mySword.transform.rotation = swordRotation;
		}
}
