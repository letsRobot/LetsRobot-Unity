#ifndef Stoppable_h
#define Stoppable_h

class Stoppable
{
   public:
      virtual ~Stoppable() noexcept { }
      virtual void Stop() = 0;
};

#endif
