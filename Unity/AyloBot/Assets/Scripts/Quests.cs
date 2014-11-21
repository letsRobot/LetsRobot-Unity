using UnityEngine;
using System.Collections.Generic;

public class Quests
{
	public void Add(string quest)
	{
		quests.Add(new Quest(quest));
		UpdateQuests();
	}
	
	public void Update(int iQuest, string quest)
	{
		if(BadIndex(iQuest))
			return;
		
		quests[iQuest].text = quest;
		UpdateQuests();
	}
	
	public void Close(int iQuest)
	{
		if(BadIndex(iQuest))
			return;
		
		quests[iQuest].closed = true;
		UpdateQuests();
	}
	
	public void Open(int iQuest)
	{
		if(BadIndex(iQuest))
			return;
		
		quests[iQuest].closed = false;
		UpdateQuests();
	}
	
	public void Remove(int iQuest)
	{
		if(BadIndex(iQuest))
			return;
		
		quests.RemoveAt(iQuest);
		UpdateQuests();
	}
	
	// This function is the one that should be changed to change the appearance of the quests.
	void UpdateQuests()
	{
		string questsString = "";
		foreach(var quest in quests)
		{
			if(quest.closed)
				questsString += "<color=#404040ff>";
			
			questsString += quest.text + "\n";
			
			if(quest.closed)
				questsString += "</color>";
		}
		
		GameObject.Find("Quests").GetComponent<TextMesh>().text = questsString;
	}
	
	bool BadIndex(int iQuest)
	{
		return iQuest < 0 || iQuest > quests.Count;
	}
	
	class Quest
	{
		public string text;
		public bool closed;
		
		public Quest(string text)
		{
			this.text = text;
			closed = false;
		}
	}
	
	IList<Quest> quests = new List<Quest>();
}
