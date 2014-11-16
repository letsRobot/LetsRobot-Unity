using System;
using System.Threading;
using System.Net.Sockets;
using System.IO;

public interface RobotMessageReceiver
{
	void NewMessage(string message);
}

public class RobotConnection
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

				lock(myLock)
				{
					if(socket != null)
					{
						try
						{
							function();
						}
						catch(Exception)
						{
							bool ignoreException = false;

							if(!ignoreException)
								ResetConnection(ref outputLock, ref hasConnected, ref otherHasConnected);
						}
					}
					else
						ResetConnection(ref outputLock, ref hasConnected, ref otherHasConnected);
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
		////
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
	volatile bool stopped = false;
	TcpClient socket;
	Thread inputThread;
	Thread outputThread;
	bool inputHasConnected = false;
	bool outputHasConnected = false;
	object connectingLock = new object();
	object inputLock = new object();
	object outputLock = new object();
	object serverPortLock = new object();
	PackageAssembler packageAssembler = new PackageAssembler();
}
