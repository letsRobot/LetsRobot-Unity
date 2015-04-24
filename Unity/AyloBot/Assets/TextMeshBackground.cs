using UnityEngine;
using System.Collections;

//[RequireComponent(typeof(MeshFilter))]
//[RequireComponent(typeof(MeshRenderer))]
public class TextMeshBackground : MonoBehaviour {

	Renderer thisTextRenderer;
	public GameObject mySpriteObject;

	// Use this for initialization
	public Vector3 center;

	GameObject textBGClone;
	void Start () {

		thisTextRenderer = this.gameObject.GetComponent<Renderer>();
		textBGClone = (GameObject)Instantiate(mySpriteObject, transform.position, Quaternion.identity);

	}

	Vector3 stepBack (Vector3 inputVec) {

		Vector3 minusZ = new Vector3(inputVec.x, inputVec.y, inputVec.z + 0.25f);
		return minusZ;

	}
	
	 //Update is called once per frame
	void Update () {

		Vector3 center = thisTextRenderer.bounds.center;
		textBGClone.transform.position = stepBack(center);
		textBGClone.transform.localScale = thisTextRenderer.bounds.size;

	}


	void drawMesh (GameObject theObj) {
		
		//Create New Mesh
		Mesh mesh = new Mesh();
		mesh.name = "TextBGMesh";
		mesh.Clear();

		//Create the Vertices
		Vector3[] vertices = new Vector3[]{

			new Vector3(-1.0f, -1.0f, 0.0f),
			new Vector3(1.0f, -1.0f, 0.0f),
			new Vector3(1.0f, 1.0f, 0.0f),
			new Vector3( -1.0f, 1.0f, 0.0f),
		};
		
		//Assign the Vertices
		mesh.vertices = vertices;
		//mesh.bounds = thisTextRenderer.bounds;

		//Draw them UVs
		mesh.uv = new Vector2[] {
			new Vector2 (0, 0),
			new Vector2 (0, 1),
			new Vector2 (1, 1),
			new Vector2 (1, 0)
		};
			
		//Normals need to be recalculated with new triangles, otherwise you get no texture yo!
		mesh.triangles = new int[] { 3, 2, 0, 2, 1, 0};
		mesh.RecalculateNormals();
		
		MeshFilter mf = theObj.gameObject.GetComponent<MeshFilter>();
		mf.mesh = mesh;
	}
}
