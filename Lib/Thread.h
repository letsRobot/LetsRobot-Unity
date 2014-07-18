#ifndef Thread_h
#define Thread_h

#include <thread>

class Thread
{
   public:
      Thread() : stopped(false)
      { }

      virtual ~Thread() noexcept
      {
         Stop();
         Join();
      }

      void Start()
      {
         const auto run =[this](){ this->Run(); };
         t = std::move(std::thread(run));
      }

      void Join()
      {
         if(t.joinable())
            t.join();
      }

      void Stop()
      {
         stopped = true;
      }

      static void Sleep(uint32_t milliseconds)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
      }

   protected:
      virtual void Run() = 0;
      volatile bool stopped;

   private:
      std::thread t;
};

#endif

