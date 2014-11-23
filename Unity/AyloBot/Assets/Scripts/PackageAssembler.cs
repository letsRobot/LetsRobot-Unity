using System.IO;

// The package format used in this class is described in UnityThreads.cpp.
public class PackageAssembler
{
	public const int packageSize = 64;

	public void AddPackage(byte[] package)
	{
		if(package.Length != packageSize)
			return;

		var input = new BinaryReader(new MemoryStream(package));

		var magic1        = input.ReadUInt32();
		var messageId     = input.ReadUInt32();
		var packageNumber = input.ReadUInt32();
		var nBytesOfData  = input.ReadByte();
		var data          = input.ReadBytes(47);
		var magic2        = input.ReadUInt32();

		if(magic1 != 0xaaaa5555 || magic2 != 0xaa55aa55 ||
           nBytesOfData > 47 ||
		   (messageId == lastMessageId && lastMessageWasBad) ||
		   (messageId == lastMessageId && packageNumber != lastPackageNumber - 1))
		{
			lastMessageWasBad = true;
			return;
		}

		if(messageId != lastMessageId)
		{
			fullMessageReceived = false;
			message = "";
		}

		for(int i = 0; i < nBytesOfData; i++)
			message += (char)data[i];

		if(packageNumber == 0)
			fullMessageReceived = true;

		lastMessageId     = messageId;
		lastPackageNumber = packageNumber;
		lastMessageWasBad = false;
	}

	public bool FullMessageWasReceived()
	{
		return fullMessageReceived;
	}

	public string GetMessage()
	{
		return message;
	}

	public bool LastMessageWasBad()
	{
		return lastMessageWasBad;
	}

	uint lastMessageId = uint.MaxValue;
	bool lastMessageWasBad = true;
	uint lastPackageNumber = 0;
	string message;
	bool fullMessageReceived = false;
}
