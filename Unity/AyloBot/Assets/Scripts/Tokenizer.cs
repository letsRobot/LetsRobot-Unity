using System;

class Tokenizer
{
	public Tokenizer(string str, char delimiter)
	{
		this.str = str;
		this.delimiter = delimiter;
	}
	
	public string GetToken()
	{
		string token = "";
		
		if(!HasMore())
			throw new Exception();
		
		while(position < str.Length && str[position] != delimiter)
		{
			token += str[position];
			position++;
		}
		
		return token;
	}
	
	public string GetString()
	{
		if(!HasMore())
			throw new Exception();
		
		return str.Substring(position);
	}
	
	public string GetString(int nChars)
	{
		if(!HasMore())
			throw new Exception();
		
		string returnString = str.Substring(position, nChars);
		position += nChars;
		return returnString;
	}
	
	public bool HasMore()
	{
		SkipDelimiters();
		
		return position < str.Length;
	}
	
	void SkipDelimiters()
	{
		while(position < str.Length && str[position] == delimiter)
			position++;
	}
	
	string str;
	char delimiter;
	int position;
}
