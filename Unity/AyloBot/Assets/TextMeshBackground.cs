using UnityEngine;
using System.Collections;

public class TextMeshBackground : MonoBehaviour {

	Renderer thisTextRenderer;
	public GameObject mySpriteObject;

	// Use this for initialization
	void Start () {

//		thisTextRenderer = this.gameObject.GetComponent<Renderer>();
//		Instantiate(mySpriteObject, transform.position, Quaternion.identity);


	
	}
	
	// Update is called once per frame
//	void Update () {
//
//		Vector3 center = thisTextRenderer.bounds.center;
//		float radius = thisTextRenderer.bounds.extents.magnitude;
//		var myBounds = thisTextRenderer.bounds.extents;
//		myBounds.x = thisTextRenderer.bounds.extents.x * 2;
//
//		mySpriteObject.transform.position = center;
//	


	
	//}

//	void OnDrawGizmosSelected () {
//		
//		Vector3 center = thisTextRenderer.bounds.center;
//		float radius = thisTextRenderer.bounds.extents.magnitude;
//		Gizmos.color = Color.white;
//		//Gizmos.DrawWireSphere(center, radius);
//		var myBounds = thisTextRenderer.bounds.extents;
//		myBounds.x = thisTextRenderer.bounds.extents.x * 2;
//		Gizmos.DrawWireCube(center, myBounds);
//		
//	}
}
