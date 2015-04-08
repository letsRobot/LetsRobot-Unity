using UnityEngine;
using System.Collections;

public class jukeBox : MonoBehaviour {

	public bool enableMusic; //enables music if true
	public AudioClip[] Songs; //All Songs
	int tracks; //Index of tracks in the JukeBox
	AudioSource thisJukeBox; //References the audio source

	bool Mute; //pauses current music
	bool nextTrack; //true if next track
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
			}

		UMusicInput(); //Unity shortcuts for Music
		newTrack();
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
			thisJukeBox.clip = Songs[playTrack];
			thisJukeBox.Play ();
			nextTrack = false;
		}
		return playTrack;
	}

	//Unity shortcuts for Music
	void UMusicInput () {

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
	}



	void Awake () {

		//Get the Audio Source assigned to this game object
		thisJukeBox = this.gameObject.GetComponent<AudioSource>();
		Mute = false;
		enableMusic = true; //This should be tied to a static global variable at some point.
		tracks = Songs.Length;
		Debug.Log("Number of Songs: " + tracks);
		nextTrack = false;
	}
	
	void Start () {
		//playMusic();
	}
	
	// Update is called once per frame
	void Update () {

		playMusic();

	}
}
