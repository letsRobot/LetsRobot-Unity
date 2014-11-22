using UnityEngine;

public class EarthQuaker
{
	public void Update()
	{
		if(!isQuaking)
			return;

		Vector3 position = originalPosition;
		if(GetDeltaTime() < durationInSeconds)
		{
			var randomOffset = Random.insideUnitSphere * magnitude;
			randomOffset.z = 0;
			position += randomOffset;
		}

		GameObject.Find("Streamer").transform.position = position;
	}
	
	public void SetDurationInSeconds(float durationInSeconds)
	{
		this.durationInSeconds = durationInSeconds;
	}
	
	public void SetMagnitude(float magnitude)
	{
		this.magnitude = magnitude;
	}

	public void StartEarthquake()
	{
		originalPosition = GameObject.Find("Streamer").transform.position;

		beginTime = Time.time;
		isQuaking = true;
	}
	
	public bool IsQuaking()
	{
		return isQuaking;
	}

	float GetDeltaTime()
	{
		return Time.time - beginTime;
	}

	Camera camera;
	float durationInSeconds = 10;
	float magnitude = 10;
	bool isQuaking = false;
	float beginTime = 0;
	Vector3 originalPosition;
}

