using System;
using System.Text;
using System.Collections.Generic;
using System.Threading;
using System.Net.Sockets;
using System.IO;

public interface RobotMessageReceiver
{
	void NewMessage(string message);
}

public interface RobotMessageSender
{
	void SendMessage(string message);
}

// The package format used in this class is described in UnityThreads.cpp.
public class RobotConnection : RobotMessageSender
{
	public RobotConnection(string server, int port, RobotMessageReceiver robotMessageReceiver)
	{
		this.server = server;
		this.port = port;
		this.robotMessageReceiver = robotMessageReceiver;

		inputThread  = new Thread(new ThreadStart(InputThread));
		outputThread = new Thread(new ThreadStart(OutputThread));
		
		inputThread.Start();
		outputThread.Start();
	}

	public void SetServer(string server, int port)
	{
		lock(serverPortLock)
		{
			this.server = server;
			this.port = port;
		}
	}

	public void Stop()
	{
		stopped = true;
		inputThread.Join();
		outputThread.Join();

		CloseSocket();
	}

	public void SendMessage(string message)
	{
		lock(sendMessagesLock)
			sendMessages.Enqueue(message);
	}

	void InputThread()
	{
		InputOutput(InputFunction, ref inputLock, ref outputLock, ref inputHasConnected, ref outputHasConnected);
	}

	void OutputThread()
	{
		InputOutput(OutputFunction, ref outputLock, ref inputLock, ref outputHasConnected, ref inputHasConnected);
	}

	void InputOutput(RobotConnectionFunction function, ref object myLock, ref object otherLock, ref bool hasConnected, ref bool otherHasConnected)
	{
		while(!stopped)
		{
			lock(connectingLock)
			{
				otherHasConnected = false;
			}

			try
			{
				lock(myLock)
				{
					if(socket == null)
						throw new Exception();

					function();
				}
			}
			catch(Exception)
			{
				ResetConnection(ref outputLock, ref hasConnected, ref otherHasConnected);
			}
		}
	}

	void ResetConnection(ref object otherLock, ref bool hasConnected, ref bool otherHasConnected)
	{
		lock(connectingLock)
		{
			if(otherHasConnected) 
				return;
			
			lock(otherLock)
			{
				if(stopped)
					return;
				
				socket = null;
				
				while(!stopped && socket == null)
				{
					try
					{
						socket = new TcpClient();
						
						IAsyncResult asyncResult;
						lock(serverPortLock)
						{
							asyncResult = socket.BeginConnect(server, port, null, null);
						}
						asyncResult.AsyncWaitHandle.WaitOne(1000);
						
						if(socket.Connected)
							socket.EndConnect(asyncResult);
						else
							throw new Exception();
						
						socket.ReceiveTimeout = 1000;
						socket.SendTimeout    = 1000;
						socket.NoDelay        = true;
						
						hasConnected = true;
					}
					catch(Exception)
					{
						CloseSocket();
					}
				}
			}
		}
	}

	void InputFunction()
	{
		byte[] package = new byte[PackageAssembler.packageSize];

		int nBytesRead = 0;
		while(nBytesRead != PackageAssembler.packageSize)
		{
			if(stopped)
				return;

			if(!socket.GetStream().DataAvailable)
			{
				Thread.Sleep(1);
				continue;
			}

			var nBytesReadThisTime = socket.GetStream().Read(package, nBytesRead, PackageAssembler.packageSize - nBytesRead);

			nBytesRead += nBytesReadThisTime;
		}

		packageAssembler.AddPackage(package);

		if(packageAssembler.LastMessageWasBad())
			throw new Exception();

		if(packageAssembler.FullMessageWasReceived())
			robotMessageReceiver.NewMessage(packageAssembler.GetMessage());
	}

	void OutputFunction()
	{
		string message;

		while(!stopped && sendMessages.Count != 0)
		{
			lock(sendMessagesLock)
				message = sendMessages.Dequeue();

			SendAsPackages(message);
		}

		var secondsSinceLastHello = DateTime.Now.Subtract(lastHelloTime).TotalSeconds;
		if(secondsSinceLastHello > 5)
		{
			SendMessage("hello");
			lastHelloTime = DateTime.Now;
		}

		Thread.Sleep(1);
	}

	void SendAsPackages(string message)
	{
		var nBytesPerPackage    = 47;
		var nPackages           = (message.Length + nBytesPerPackage - 1) / nBytesPerPackage;
		var packageNumber       = nPackages - 1;
		var nBytesLeftInMessage = message.Length;
		byte[] dataZeros        = new byte[nBytesPerPackage]; // Elements are initialized to 0.

		messageId++;

		while(nBytesLeftInMessage != 0)
		{
			uint magic1     = 0xaaaa5555;
			uint magic2     = 0xaa55aa55;
			byte nBytesData = (byte)(nBytesLeftInMessage >= nBytesPerPackage ? nBytesPerPackage : nBytesLeftInMessage);
			var data        = message.Substring(message.Length - nBytesLeftInMessage);

			socket.GetStream().Write(BitConverter.GetBytes(magic1),        0, 4);
			socket.GetStream().Write(BitConverter.GetBytes(messageId),     0, 4);
			socket.GetStream().Write(BitConverter.GetBytes(packageNumber), 0, 4);
			socket.GetStream().Write(BitConverter.GetBytes(nBytesData),    0, 1);
			socket.GetStream().Write(Encoding.UTF8.GetBytes(data),         0, nBytesData);
			socket.GetStream().Write(dataZeros,                            0, nBytesPerPackage - nBytesData);
			socket.GetStream().Write(BitConverter.GetBytes(magic2),        0, 4);

			packageNumber--;
			nBytesLeftInMessage -= nBytesData;
		}
	}

	void CloseSocket()
	{
		if(socket != null)
			socket.Close();
		
		socket = null;
	}

	delegate void RobotConnectionFunction();

	string server;
	int port;
	RobotMessageReceiver robotMessageReceiver;
	volatile bool stopped             = false;
	TcpClient socket;
	Thread inputThread;
	Thread outputThread;
	bool inputHasConnected            = false;
	bool outputHasConnected           = false;
	object connectingLock             = new object();
	object inputLock                  = new object();
	object outputLock                 = new object();
	object serverPortLock             = new object();
	PackageAssembler packageAssembler = new PackageAssembler();
	object sendMessagesLock           = new object();
	Queue<string> sendMessages        = new Queue<string>();
	int messageId                     = 0;
	DateTime lastHelloTime            = DateTime.Now;
}
