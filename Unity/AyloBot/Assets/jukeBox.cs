using UnityEngine;
using System.Collections;

public class jukeBox : MonoBehaviour {

	public bool enableMusic; //enables music if true
	public AudioClip[] Songs; //All Songs
	int tracks; //Index of tracks in the JukeBox
	AudioSource thisJukeBox; //References the audio source

	bool Mute; //pauses current music
	bool nextTrack; //true if next track
	bool skipTrack; //True if skipping to next track
	int playTrack = -1; //index of next track to play

	float songLength; //Length of the current song

	//Play Music
	void playMusic() {

		if (enableMusic == true && !thisJukeBox.isPlaying) {
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

	int newTrack() {

		int trackIndex = tracks - 1;
		if (!thisJukeBox.isPlaying) {
			nextTrack = true;
		}

		if (nextTrack == true) {
			playTrack += 1;
			if (playTrack > trackIndex) {
				playTrack = 0;
			}
			nextTrack = false;
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
		}
	}



	void Awake () {

		//Get the Audio Source assigned to this game object
		thisJukeBox = this.gameObject.GetComponent<AudioSource>();
		Mute = false;
		enableMusic = true; //This should be tied to a static global variable at some point.
		tracks = Songs.Length;
		Debug.Log("Number of Songs: " + tracks);
		nextTrack = false;
		skipTrack = false;
	}
	
	void Start () {

	}
	
	// Update is called once per frame
	void Update () {

		playMusic();

	}
}
