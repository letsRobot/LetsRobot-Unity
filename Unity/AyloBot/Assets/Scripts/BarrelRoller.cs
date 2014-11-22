using UnityEngine;

public class BarrelRoller
{
	public void Update()
	{
		if(!isRolling)
			return;

		var angle = GetDeltaTime() * 360.0f / secondsPerRotation;

		if(angle >= 360)
		{
			isRolling = false;
			angle = 0;
		}

		GameObject.Find("Streamer").transform.rotation = Quaternion.Euler(0, 0, angle) * originalRotation;
	}

	public void SetSecondsPerRotation(float secondsPerRotation)
	{
		this.secondsPerRotation = secondsPerRotation;
	}

	public void StartBarrelRoll()
	{
		originalRotation = GameObject.Find("Streamer").transform.rotation;
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

	Quaternion originalRotation;
	float secondsPerRotation = 10;
	bool isRolling = false;
	float beginTime = 0;
}
