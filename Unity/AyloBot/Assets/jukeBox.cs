using UnityEngine;
using System.Collections;

public class jukeBox : MonoBehaviour {
	
	public static bool getCommand = false; //true if robot sends command

	//Manages music commands from the robot
	void robotMusicController () {

		if (getCommand == true) {

			enableMusic = RobotEnableMusic;
			Mute = RobotMute;
			loopTrack = RobotLoopTrack;

			skipTrack = RobotSkipTrack;
			if (skipTrack == true) {
				triggerNextTrack = true;
			}

			backTrack = RobotBackTrack;
			if (backTrack == true) {
				thisJukeBox.Stop();
			}
			restartTrack = RobotRestartTrack;
			if (restartTrack == true) {
				thisJukeBox.Stop();
			}

			getCommand = false; //end command execution

		} else {

			//Reset Varialbes after getting commands
			RobotEnableMusic = enableMusic;
			RobotMute = Mute;
			RobotLoopTrack = loopTrack;
			RobotSkipTrack = skipTrack;
			RobotBackTrack = backTrack;
			RobotRestartTrack = restartTrack;

		}
	}

	public bool enableMusic; //enables music if true
	public static bool RobotEnableMusic = false;

	public AudioClip[] Songs; //All Songs
	int tracks; //Index of tracks in the JukeBox
	AudioSource thisJukeBox; //References the audio source of this game Unity Game Object
	bool nextTrack; //returns true if jukebox is ready for the next track
	bool triggerNextTrack;//Triggers next track if song is looping

	public bool loopTrack;//loops the current track
	public static bool RobotLoopTrack;
	bool Mute; //Mutes the current track
	public static bool RobotMute;
	bool skipTrack; //Skip to the next track
	public static bool RobotSkipTrack;
	bool backTrack; //True if skpping backrward
	public static bool RobotBackTrack;
	bool restartTrack; //trickers restarting a track
	public static bool RobotRestartTrack;

	int playTrack = -1; //index of next track to play
	float songLength; //Length of the current song
	
	void Awake () {

		//Get the Audio Source assigned to this game object
		thisJukeBox = this.gameObject.GetComponent<AudioSource>();
		Mute = false;
		enableMusic = true; //This should be tied to a static global variable at some point.
		tracks = Songs.Length;
		Debug.Log("Number of Songs: " + tracks);
		nextTrack = false;
		skipTrack = false;
		loopTrack = false;
		triggerNextTrack = false;
		restartTrack = false;
		backTrack = false;
	}
	
	void Start () {

	}
	
	// Update is called once per frame
	void Update () {

		//If music is enabled, play music, otherwise DONT!
		robotMusicController();
		if (enableMusic == true) {
			playMusic();
		} else {
			thisJukeBox.Stop();
		}
	}

	//Play Music
	void playMusic() {
		
		if (!thisJukeBox.isPlaying) {
			//pick a song and play it!
			AudioClip currentSong = Songs[newTrack()];
			thisJukeBox.clip = currentSong;
			thisJukeBox.Play ();
			Debug.Log("Song is Playing");
			songLength = currentSong.length;
			Debug.Log ("Track Length: " + songLength);
			
			//skip to the next song if "skipped"
		} else if (skipTrack == true && thisJukeBox.isPlaying) {
			thisJukeBox.Stop();
			Debug.Log("Skipping to next track");
			skipTrack = false;
		}
		
		UMusicInput(); //Unity shortcuts for Music
	}
	
	//In charge of track switching!
	int newTrack() {
		
		int trackIndex = tracks - 1;
		if (!thisJukeBox.isPlaying && loopTrack == false) {
			nextTrack = true;
		} else if (loopTrack == true && triggerNextTrack == true) {
			nextTrack = true;
		}
		
		if (nextTrack == true) {
			if (restartTrack == true) {
				restartTrack = false;
			} else if (backTrack == true) {
				playTrack -= 1;
				backTrack = false;
				if (playTrack < 0) {
					playTrack = trackIndex;
				}
			} else {
				playTrack += 1;
			}
			if (playTrack > trackIndex) {
				playTrack = 0;
			}
			nextTrack = false;
			triggerNextTrack = false;
		}
		
		return playTrack;
	}
	
	//Unity shortcuts for Music
	void UMusicInput () {
		
		//Press M to Mute the current track
		if (Input.GetKeyDown(KeyCode.M) && Mute == false) {
			Mute = true;
		} 
		else if ( Input.GetKeyDown(KeyCode.M) && Mute == true) {
			Mute = false;
		}
		
		if (Mute == true && thisJukeBox.mute == false) {
			thisJukeBox.mute = true;
		}
		
		if (Mute == false && thisJukeBox.mute == true) {
			thisJukeBox.mute = false;
		}
		
		//Press Right Arrow to skip to the next track.
		if (Input.GetKeyDown (KeyCode.RightArrow) && thisJukeBox.isPlaying && skipTrack == false) {
			skipTrack = true;
			triggerNextTrack = true;
		}
		
		if (Input.GetKeyDown (KeyCode.DownArrow) && thisJukeBox.isPlaying && restartTrack == false) {
			restartTrack = true;
			thisJukeBox.Stop();
		}
		
		if (Input.GetKeyDown (KeyCode.LeftArrow) && thisJukeBox.isPlaying && backTrack == false) {
			backTrack = true;
			thisJukeBox.Stop ();
		}
	}
}
