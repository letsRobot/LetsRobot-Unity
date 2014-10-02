#ifndef ThreadSafeQueue_h
#define ThreadSafeQueue_h

#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue
{
   public:
      void Push(const T & element)
      {
         assert(&element);

         std::lock_guard<std::mutex> l(lock);
         queue.push(element);
      }

      T Pop()
      {
         std::lock_guard<std::mutex> l(lock);

         assert(!queue.empty());

         T element = std::move(queue.front());
         queue.pop();

         return std::move(element);
      }

      bool IsEmpty()
      {
         std::lock_guard<std::mutex> l(lock);
         return queue.empty();
      }

   private:
      std::mutex lock;
      std::queue<T> queue;
};

#endif
