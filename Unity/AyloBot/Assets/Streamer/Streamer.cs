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

      CloseSocket();
   }

   public void SetServer(String address, int port)
   {
      Monitor.Enter(streamLock);
      this.address = address;
      this.port = port;
      Monitor.Exit(streamLock);
   }

   public void SetMaxSize(int maxSize)
   {
      this.maxSize = maxSize;
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
         Connect();
         ReceiveData();
      }
   }

   void Connect()
   {
      while(!stopped && socket == null)
      {
         try
         {
            socket = new TcpClient();

            Monitor.Enter(streamLock);
            var asyncResult = socket.BeginConnect(address, port, null, null);
            Monitor.Exit(streamLock);
            asyncResult.AsyncWaitHandle.WaitOne(1000);

            if(socket.Connected)
               socket.EndConnect(asyncResult);
            else
               throw new Exception();

            socket.ReceiveTimeout = 1000;
         }
         catch(Exception)
         {
            CloseSocket();
            continue;
         }
      }
   }

   void ReceiveData()
   {
      while(!stopped && socket != null)
      {
         try
         {
            var magicNumber1 = ReadInt();
            var size         = ReadInt();
            var magicNumber2 = ReadInt();
            var size2        = ReadInt();
            var magicNumber3 = ReadInt();

            if(magicNumber1 != 0x1234 || magicNumber2 != 0x5678 || magicNumber3 != 0xabcd || size != size2 || size > maxSize)
               throw new Exception();

            if(size > inputBufferSize)
            {
               inputBuffer = new byte[size];
               inputBufferSize = size;
            }
            
            int totalBytesRead = 0;
            while(totalBytesRead < size && !stopped)
            {
               var nBytesRead = socket.GetStream().Read(inputBuffer, totalBytesRead, size - totalBytesRead);
               totalBytesRead += nBytesRead;
               
               if(nBytesRead == 0)
                  throw new Exception();
            }

            SwapBuffers();
         }
         catch(Exception)
         {
            CloseSocket();
            return;
         }
      }
   }

   int ReadInt()
   {
      socket.GetStream().Read(intBuffer, 0, 4);
      return System.BitConverter.ToInt32(intBuffer, 0);
   }

   void SwapBuffers()
   {
      Monitor.Enter(streamLock);
      try
      {
         var tempBuffer = inputBuffer;
         inputBuffer    = outputBuffer;
         outputBuffer   = tempBuffer;
         
         var tempBufferSize = inputBufferSize;
         inputBufferSize    = outputBufferSize;
         outputBufferSize   = tempBufferSize;

         newBufferIsReady = true;
      }
      finally
      {
         Monitor.Exit(streamLock);
      }
   }

   void CloseSocket()
   {
      if(socket != null)
         socket.Close();
      
      socket = null;
   }

   Thread thread;
   object streamLock = new object();
   TcpClient socket;
   volatile bool stopped = false;
   byte[] intBuffer = new byte[4];
   volatile int inputBufferSize = -1;
   volatile int outputBufferSize = -1;
   volatile byte[] inputBuffer;
   volatile byte[] outputBuffer;
   volatile bool newBufferIsReady = false;
   int maxSize = 10 * 1024 * 1024;
   String address;
   int port;
}

public class Streamer : MonoBehaviour
{
   public String address;
   public int port;
   public int maxFileSize; // If the received file size is larger than maxFileSize it is assumed that the data has been corrupted.

   void Start()
   {
      texture1 = new Texture2D(1, 1);
      texture2 = new Texture2D(1, 1);

      texture1.SetPixel(0, 0, new Color(0, 0, 0));
      texture2.SetPixel(0, 0, new Color(0, 0, 0));

      texture1.Apply();
      texture2.Apply();

		address = Constants.IP1;
		port = Constants.Port1;
   }

   void Update()
   {
      if(stream == null)
      {
         stream = new StreamerThread();
         stream.SetServer(address, port);
         stream.SetMaxSize(maxFileSize);
         stream.Start();
      }

      stream.SetMaxSize(maxFileSize);
      stream.SetServer(address, port);

      var buffer = stream.GetBuffer();
      if(buffer != null)
      {
         texture2.LoadImage(buffer);

         // Check if the texture has the same dimensions as the default texture Unity uses when LoadImage() fails.
         if(texture2.height != 8 || texture2.width != 8)
         {
            var tempTexture = texture1;
            texture1 = texture2;
            texture2 = tempTexture;
         }

         UpdateFrameRate();
         UpdateAspectRatio();
      }

      stream.DoneWithBuffer();

      gameObject.GetComponent<MeshRenderer>().material.SetTexture(0, texture1);

		//I can't seem to reset the stream : D
		if (InputData.resetStreamer == true) {
			stream.Stop();
			InputData.resetStreamer = false;
		}
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
      if(texture1.height == 0)
         return;

      float textureAspectRatio = texture1.width / (float)texture1.height;

      var transform = gameObject.transform;
      transform.localScale = new Vector3(textureAspectRatio, 0, 1);
   }

   int frames = 0;
   float frameCountStartTime = 0;
   StreamerThread stream;
   Texture2D texture1;
   Texture2D texture2;
}
