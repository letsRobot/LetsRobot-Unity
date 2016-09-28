using UnityEngine;
using System.Collections;

public static class Constants {


	//Variables that are universal.

	//Network stuff
	public static string IP1 = "192.168.1.999";
	public static int Port1 = 1234;
	public static int UnityPort1 = 40000;

	// has to be constants/global so skyNet & Robot connections can get at it
	public static RobotStuff roboStuff=new RobotStuff();
	// has to be constants/global so Robot class & skyNet can get at it
	public static RobotMessages skyNetMessages=new RobotMessages("64.185.234.177", 40100);
	// moved out of robot class
	public static string openQuestColor="";
	public static string closedQuestColor="";

	//Unity stuff
	public static bool TD = false;
	public static bool updateTD = false;
	public static bool standBy = false;
	public static bool updateStandBy = false;
	public static bool gameOver = false;
	public static bool updateGameOver = false;

	//Robot stuff
	public static bool telly = true;
	public static bool robotLive = false;
	public static bool gripperClose = true;
	
	//Robot Variables-------------------------------------------

	public static string robotName = "";

	//IMU Variables
	public static Vector3 imuEuler;
	//Try converting the quaternion to Euler instead of using the Eulers from the IMU...
	public static Quaternion imuQuaternion;
	public static float imuTemp = 0.0f;

	public static int headPan;
	public static int headTilt;


	
}

