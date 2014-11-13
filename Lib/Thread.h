#ifndef Thread_h
#define Thread_h

#include <thread>
#include <exception>

class Thread
{
   public:
      Thread()
         : stopped(false),
           badAlloc(false)
      { }

      virtual ~Thread()
      {
         Stop();
         Join();
      }

      void Start()
      {
         const auto run =  [this]()
                           {
                              this->RunAndCatchExceptions();
                           };

         t = std::move(std::thread(run));
      }

      void Join()
      {
         if(t.joinable())
            t.join();
      }

      virtual void Stop()
      {
         stopped = true;
      }

      static void Sleep(uint32_t milliseconds)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
      }

      void RethrowException()
      {
         if(exceptionPointer)
            std::rethrow_exception(exceptionPointer);

         if(badAlloc)
            throw std::bad_alloc();
      }

   protected:
      virtual void Run() = 0;
      volatile bool stopped;

   private:
      void RunAndCatchExceptions()
      {
         try
         {
            Run();
         }
         catch(...)
         {
            try
            {
               exceptionPointer = std::current_exception();
            }
            catch(std::bad_alloc &)
            {
               badAlloc = true;
            }
         }
      }

      std::thread t;
      std::exception_ptr exceptionPointer;
      bool badAlloc;
};

#endif
