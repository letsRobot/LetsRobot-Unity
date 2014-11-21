#ifndef Stoppable_h
#define Stoppable_h

class Stoppable
{
   public:
      virtual ~Stoppable() { }
      virtual void Stop() = 0;
};

#endif
