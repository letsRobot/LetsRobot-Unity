#ifndef ErrorObserver_h
#define ErrorObserver_h

class ErrorObserver
{
   public:
      virtual ~ErrorObserver() noexcept { }
      virtual void ReportFatalError() = 0;
};

#endif
