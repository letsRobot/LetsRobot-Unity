using UnityEngine;
using System.Collections.Generic;

public class Quests
{
	public Quests()
	{
		var robot = GameObject.Find("Robot").GetComponent<Robot>();
		openColor = robot.openQuestColor;
		closedColor = robot.closedQuestColor;
	}

	public void Add(string quest)
	{
		quests.Add(new Quest(quest));
		UpdateQuests();
	}
	
	public void Update(int iQuest, string quest)
	{
		if(IsBadIndex(iQuest))
			return;
		
		quests[iQuest].text = quest;
		UpdateQuests();
	}
	
	public void Close(int iQuest)
	{
		if(IsBadIndex(iQuest))
			return;
		
		quests[iQuest].closed = true;
		UpdateQuests();
	}
	
	public void Open(int iQuest)
	{
		if(IsBadIndex(iQuest))
			return;
		
		quests[iQuest].closed = false;
		UpdateQuests();
	}
	
	public void Remove(int iQuest)
	{
		if(IsBadIndex(iQuest))
			return;
		
		quests.RemoveAt(iQuest);
		UpdateQuests();
	}
	
	// Change this function to change the appearance of the quests.
	void UpdateQuests()
	{
		string questsString = "";
		foreach(var quest in quests)
		{
			var color = openColor;

			if(quest.closed)
				color = closedColor;
			
			questsString += "<color=#" + color + ">";

			questsString += quest.text + "\n";
			
			questsString += "</color>";
		}
		
		GameObject.Find("Quests").GetComponent<TextMesh>().text = questsString;
	}
	
	bool IsBadIndex(int iQuest)
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

	string openColor;
	string closedColor;
	IList<Quest> quests = new List<Quest>();
}
