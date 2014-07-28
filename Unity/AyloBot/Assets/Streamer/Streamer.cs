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

            var asyncResult = socket.BeginConnect(address, port, null, null);
            asyncResult.AsyncWaitHandle.WaitOne(1000);

            if(!socket.Connected)
               throw new Exception();

            // We really should call socket.EndConnect() here.
            // It was removed as it seemed to make the socket hang when the WiFi connection was bad.

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

   void SwapBuffers()
   {
      Monitor.Enter(streamLock);
      try
      {
         var tempBuffer = inputBuffer;
         inputBuffer = outputBuffer;
         outputBuffer = tempBuffer;
         
         var tempBufferSize = inputBufferSize;
         inputBufferSize = outputBufferSize;
         outputBufferSize = tempBufferSize;

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
   byte[] sizeBuffer = new byte[4];
   volatile int inputBufferSize = -1;
   volatile int outputBufferSize = -1;
   volatile byte[] inputBuffer;
   volatile byte[] outputBuffer;
   volatile bool newBufferIsReady = false;
   String address;
   int port;
}

public class Streamer : MonoBehaviour
{
   public String address;
   public int port;

   void Start()
   {
      texture1 = new Texture2D(1, 1);
      texture2 = new Texture2D(1, 1);

      texture1.SetPixel(0, 0, new Color(0, 0, 0));
      texture2.SetPixel(0, 0, new Color(0, 0, 0));

      texture1.Apply();
      texture2.Apply();
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
         if(texture2.LoadImage(buffer))
         {
            var tempTexture = texture1;
            texture1 = texture2;
            texture2 = tempTexture;
         }
         stream.DoneWithBuffer();

         UpdateFrameRate();
         UpdateAspectRatio();
      }

      gameObject.GetComponent<MeshRenderer>().material.SetTexture(0, texture1);
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

   private int frames = 0;
   private float frameCountStartTime = 0;
   private StreamerThread stream;
   private Texture2D texture1;
   private Texture2D texture2;
}
