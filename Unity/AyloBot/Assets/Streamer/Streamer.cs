using UnityEngine;
using System;
using System.Threading;
using System.Net.Sockets;
using System.IO;

public class StreamerThread
{
	public StreamerThread()
	{
		thread = new Thread(new ThreadStart(Stream));
	}

	public void Start()
	{
		thread.Start();
	}

	public void Stop()
	{
		stopped = true;
		thread.Join();

		if(socket != null)
			socket.Close();
	}

	public void SetServer(String address, int port)
	{
		this.address = address;
		this.port = port;
	}

	public byte[] GetBuffer()
	{
		if(!newBufferIsReady)
			return null;

		Monitor.Enter(streamLock);
		newBufferIsReady = false;

		return outputBuffer;
	}

	public void DoneWithBuffer()
	{
		Monitor.Exit(streamLock);
	}

	public void Stream()
	{		
		while(!stopped)
		{
			Thread.Sleep(10);

			try
			{
				if(socket == null)
				{
					socket = new TcpClient(address, port);
					socket.ReceiveTimeout = 1000;
				}
			}
			catch(SocketException)
			{
				continue;
			}

			try
			{
				socket.GetStream().Read(sizeBuffer, 0, 4);
				var size = System.BitConverter.ToInt32(sizeBuffer, 0);

				if(size > inputBufferSize)
				{
					inputBuffer = new byte[size];
					inputBufferSize = size;
				}

				int totalBytesRead = 0;
				while(totalBytesRead < size)
				{
					var nBytesRead = socket.GetStream().Read(inputBuffer, totalBytesRead, size - totalBytesRead);
					totalBytesRead += nBytesRead;

					if(nBytesRead == 0)
					{
						socket.Close();
						socket = null;
						break;
					}
				}

				Monitor.Enter(streamLock);
				try
				{
					SwapBuffers();
					newBufferIsReady = true;
				}
				finally
				{
					Monitor.Exit(streamLock);
				}
			}
			catch(SocketException)
			{
				socket.Close();
				socket = null;
				continue;
			}
			catch(Exception e)
			{
				socket.Close();
				socket = null;
				continue;
			}
		}
	}

	void SwapBuffers()
	{
		var tempBuffer = inputBuffer;
		inputBuffer = outputBuffer;
		outputBuffer = tempBuffer;
		
		var tempBufferSize = inputBufferSize;
		inputBufferSize = outputBufferSize;
		outputBufferSize = tempBufferSize;
	}

	private Thread thread;
	private object streamLock = new object();
	private TcpClient socket;
	private volatile bool stopped = false;
	byte[] sizeBuffer = new byte[4];
	private volatile int inputBufferSize = -1;
	private volatile int outputBufferSize = -1;
	private volatile byte[] inputBuffer;
	private volatile byte[] outputBuffer;
	private volatile bool newBufferIsReady = false;
	private String address;
	private int port;
}

public class Streamer : MonoBehaviour
{
	public String address;
	public int port;

	void Start()
	{
		texture = new Texture2D(1, 1);
		texture.SetPixel(0, 0, new Color(0, 0, 0));
		texture.Apply();
	}

	void Update()
	{
		if(stream == null)
		{
			stream = new StreamerThread();
			stream.SetServer(address, port);
			stream.Start();
		}

		stream.SetServer(address, port);

		var buffer = stream.GetBuffer();
		if(buffer != null)
		{
			texture.LoadImage(buffer);
			stream.DoneWithBuffer();

			UpdateFrameRate();
			UpdateAspectRatio();
		}

		gameObject.GetComponent<MeshRenderer>().material.SetTexture(0, texture);
	}

	public void OnApplicationQuit()
	{
		stream.Stop();
	}

	void UpdateFrameRate()
	{
		if(Time.time - frameCountStartTime > 1)
		{
			frameCountStartTime = Time.time;
			frames = 0;
		}
		else
			frames++;
	}

	void UpdateAspectRatio()
	{
		if(texture.height == 0)
			return;

		float textureAspectRatio = texture.width / (float)texture.height;

		var transform = gameObject.transform;
		transform.localScale = new Vector3(textureAspectRatio, 0, 1);
	}

	private int frames = 0;
	private float frameCountStartTime = 0;
	private StreamerThread stream;
	private Texture2D texture;
}
