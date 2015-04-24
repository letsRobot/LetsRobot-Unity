// CONFIDENTIAL Copyright 2013 (C) Little Polygon LLC, All Rights Reserved.

using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using Diagnostics = System.Diagnostics;

// Base class to use instead of MonoBehaviour.  No additional cost (all methods are static), but
// hella convenient, bro.
public class CustomBehaviour : MonoBehaviour {
	
	// TAU MANIFESTO!
	public static float tau = Mathf.PI + Mathf.PI;
	
	// Shader-Style Vector Shorthand
	public static Vector2 vec(float x, float y) { return new Vector2(x, y); }
	public static Vector3 vec(float x, float y, float z) { return new Vector3(x, y, z); }
	public static Vector3 vec(Vector2 v, float z) { return new Vector3(v.x, v.y, z); }
	public static Vector4 vec(float x, float y, float z, float w) { return new Vector4(x, y, z, w); }
	public static Vector4 vec(Vector3 v, float w) { return new Vector4(v.x, v.y, v.z, w); }
		
	// Easy way to type rectangles
	public static UnityEngine.Rect rect(float x, float y, float w, float h) { return new Rect(x, y, w, h); }
	public static UnityEngine.Rect rect(Vector2 p, Vector2 s) { return new Rect(p.x, p.y, s.x, s.y); }
	
	// Runtime assertions which only run in the editor
	[Diagnostics.Conditional ("UNITY_ENGINE")]
	public static void Assert(bool cond) { 
		if (!cond) {
			Debug.LogError("Assertion Failed, Dawg");
			Application.Quit();
		}
	}
	
	// Some easing functions
	public static float Parabola(float x) { return 1f - (x=1f-x-x)*(x); }
	public static float ParabolaDeriv(float x) { return 4f*(1f-x-x); }
	public static float EaseOut2(float u) { return 1f-(u=1f-u)*u; }
	public static float EaseOut4(float u) { return 1f-(u=1f-u)*u*u*u; }
	
	// Generic versions of Unity calls that take a type argument
	public static T Dup<T> (T obj) where T : UnityEngine.Object { return Instantiate(obj) as T; }
	public static T Dup<T> (T obj, Vector3 pos) where T : UnityEngine.Object { return Instantiate(obj, pos, Quaternion.identity) as T; }
	public static T Dup<T> (T obj, Vector3 pos, Quaternion q) where T : UnityEngine.Object { return Instantiate(obj, pos, q) as T; }
	
	public static T LoadResource<T> (string name) where T : UnityEngine.Object { return Resources.Load(name, typeof(T)) as T; }
	public static T CreateInstance<T> (string name) where T : UnityEngine.Object { return Dup<T>(LoadResource<T>(name)); }
	
	public static T FindObject<T> () where T : UnityEngine.Object { return GameObject.FindObjectOfType(typeof(T)) as T; }
	public static IEnumerable<T> FindObjects<T>() where T : UnityEngine.Object { 
		foreach(T elem in GameObject.FindObjectsOfType(typeof(T))) {
			yield return elem;
		}
	}
	
	// Destroy method which is sure to null the reference (coroutines or closures might keep them around)
	public static void DestroyGameObject<T>(ref T script) where T : MonoBehaviour {
		Destroy(script.gameObject);
		script = null;
	}
	
	// LISP-like map
	public static R[] Map<D,R>(Func<D,R> f, D[] li) {
		int len = li.Length;
		var result = new R[len];
		for(int i=0; i<len; ++i) {
			result[i] = f(li[i]);
		}
		return result;
	}
	
	// More semantic way to say "the first one"
	public static bool TakeAny<T>(IEnumerable<T> coll, out T t) {
		var i = coll.GetEnumerator();
		if (i.MoveNext()) {
			t = i.Current;
			return true;
		} else {
			t = default(T);
			return false;
		}
	}
	
	// Prolly in the std lib, but whatever
	public static void Swap<T>(ref T u, ref T v) {
		var tmp = u;
		u = v;
		v = tmp;
	}
	
	// Easy color literals
	public static Color rgb(uint hex) {
		return new Color(
			((0xff0000 & hex) >> 16) / 255f,
			((0x00ff00 & hex) >>  8) / 255f,
			((0x0000ff & hex)      ) / 255f
		);
			
	}
	public static Color rgb(float r, float g, float b) { return new Color(r, g, b); }
	public static Color rgba(float r, float g, float b, float a) { return new Color(r, g, b, a); }
	public static Color rgba(Color c, float a) { return new Color(c.r, c.g, c.b, a); }
	
	public static IEnumerable<float> interpolate(float duration) {
		for(var t=0f; t<duration; t+=Time.deltaTime) {
			yield return t / duration;
		}
		yield return 1f;
	}
}


