using UnityEngine;

public class BarrelRoller
{
	public void Update()
	{
		if(!isRolling)
			return;

		var degrees = GetDeltaTime() * 360.0f / secondsPerRotation;

		if(degrees >= 360)
		{
			isRolling = false;
			degrees = 0;
		}

		var camera = GameObject.Find("Main Camera").GetComponent<Camera>();
		camera.transform.rotation = Quaternion.Euler(0, 0, degrees);
	}

	public void SetSecondsPerRotation(float secondsPerRotation)
	{
		this.secondsPerRotation = secondsPerRotation;
	}

	public void StartBarrelRoll()
	{
		beginTime = Time.time;
		isRolling = true;
	}

	public bool IsRolling()
	{
		return isRolling;
	}

	float GetDeltaTime()
	{
		return Time.time - beginTime;
	}

	float secondsPerRotation = 10;
	bool isRolling = false;
	float beginTime = 0;
}
