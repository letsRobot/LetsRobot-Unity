#include <netdb.h>
#include <iostream>
#include <sstream>
#include <errno.h>

std::string ErrnoToString()
{
   // This is written as a series of else if statements instead of a switch
   // because on some systems some of the error codes can have the same value.

   if(errno == E2BIG)
      return "E2BIG";

   else if(errno == EACCES)
      return "EACCES";

   else if(errno == EADDRINUSE)
      return "EADDRINUSE";

   else if(errno == EADDRNOTAVAIL)
      return "EADDRNOTAVAIL";

   else if(errno == EAFNOSUPPORT)
      return "EAFNOSUPPORT";

   else if(errno == EAGAIN)
      return "EAGAIN";

   else if(errno == EALREADY)
      return "EALREADY";

   else if(errno == EBADF)
      return "EBADF";

   else if(errno == EBADMSG)
      return "EBADMSG";

   else if(errno == EBUSY)
      return "EBUSY";

   else if(errno == ECANCELED)
      return "ECANCELED";

   else if(errno == ECHILD)
      return "ECHILD";

   else if(errno == ECONNABORTED)
      return "ECONNABORTED";

   else if(errno == ECONNREFUSED)
      return "ECONNREFUSED";

   else if(errno == ECONNRESET)
      return "ECONNRESET";

   else if(errno == EDEADLK)
      return "EDEADLK";

   else if(errno == EDESTADDRREQ)
      return "EDESTADDRREQ";

   else if(errno == EDOM)
      return "EDOM";

   else if(errno == EDQUOT)
      return "EDQUOT";

   else if(errno == EEXIST)
      return "EEXIST";

   else if(errno == EFAULT)
      return "EFAULT";

   else if(errno == EFBIG)
      return "EFBIG";

   else if(errno == EHOSTUNREACH)
      return "EHOSTUNREACH";

   else if(errno == EIDRM)
      return "EIDRM";

   else if(errno == EILSEQ)
      return "EILSEQ";

   else if(errno == EINPROGRESS)
      return "EINPROGRESS";

   else if(errno == EINTR)
      return "EINTR";

   else if(errno == EINVAL)
      return "EINVAL";

   else if(errno == EIO)
      return "EIO";

   else if(errno == EISCONN)
      return "EISCONN";

   else if(errno == EISDIR)
      return "EISDIR";

   else if(errno == ELOOP)
      return "ELOOP";

   else if(errno == EMFILE)
      return "EMFILE";

   else if(errno == EMLINK)
      return "EMLINK";

   else if(errno == EMSGSIZE)
      return "EMSGSIZE";

   else if(errno == EMULTIHOP)
      return "EMULTIHOP";

   else if(errno == ENAMETOOLONG)
      return "ENAMETOOLONG";

   else if(errno == ENETDOWN)
      return "ENETDOWN";

   else if(errno == ENETRESET)
      return "ENETRESET";

   else if(errno == ENETUNREACH)
      return "ENETUNREACH";

   else if(errno == ENFILE)
      return "ENFILE";

   else if(errno == ENOBUFS)
      return "ENOBUFS";

   else if(errno == ENODATA)
      return "ENODATA";

   else if(errno == ENODEV)
      return "ENODEV";

   else if(errno == ENOENT)
      return "ENOENT";

   else if(errno == ENOEXEC)
      return "ENOEXEC";

   else if(errno == ENOLCK)
      return "ENOLCK";

   else if(errno == ENOLINK)
      return "ENOLINK";

   else if(errno == ENOMEM)
      return "ENOMEM";

   else if(errno == ENOMSG)
      return "ENOMSG";

   else if(errno == ENOPROTOOPT)
      return "ENOPROTOOPT";

   else if(errno == ENOSPC)
      return "ENOSPC";

   else if(errno == ENOSR)
      return "ENOSR";

   else if(errno == ENOSTR)
      return "ENOSTR";

   else if(errno == ENOSYS)
      return "ENOSYS";

   else if(errno == ENOTCONN)
      return "ENOTCONN";

   else if(errno == ENOTDIR)
      return "ENOTDIR";

   else if(errno == ENOTEMPTY)
      return "ENOTEMPTY";

   else if(errno == ENOTSOCK)
      return "ENOTSOCK";

   else if(errno == ENOTSUP)
      return "ENOTSUP";

   else if(errno == ENOTTY)
      return "ENOTTY";

   else if(errno == ENXIO)
      return "ENXIO";

   else if(errno == EOPNOTSUPP)
      return "EOPNOTSUPP";

   else if(errno == EOVERFLOW)
      return "EOVERFLOW";

   else if(errno == EPERM)
      return "EPERM";

   else if(errno == EPIPE)
      return "EPIPE";

   else if(errno == EPROTO)
      return "EPROTO";

   else if(errno == EPROTONOSUPPORT)
      return "EPROTONOSUPPORT";

   else if(errno == EPROTOTYPE)
      return "EPROTOTYPE";

   else if(errno == ERANGE)
      return "ERANGE";

   else if(errno == EROFS)
      return "EROFS";

   else if(errno == ESPIPE)
      return "ESPIPE";

   else if(errno == ESRCH)
      return "ESRCH";

   else if(errno == ESTALE)
      return "ESTALE";

   else if(errno == ETIME)
      return "ETIME";

   else if(errno == ETIMEDOUT)
      return "ETIMEDOUT";

   else if(errno == ETXTBSY)
      return "ETXTBSY";

   else if(errno == EWOULDBLOCK)
      return "EWOULDBLOCK";

   else if(errno == EXDEV)
      return "EXDEV";

   else
   {
      std::stringstream str;
      str << "Unknown (errno = " << errno << ")";
      return str.str();
   }
}

std::string NetdbErrorToString(int errorCode)
{
   switch(errorCode)
   {
      case EAI_AGAIN:
         return "EAI_AGAIN";

      case EAI_BADFLAGS:
         return "EAI_BADFLAGS";

      case EAI_FAIL:
         return "EAI_FAIL";

      case EAI_FAMILY:
         return "EAI_FAMILY";

      case EAI_MEMORY:
         return "EAI_MEMORY";

      case EAI_NONAME:
         return "EAI_NONAME";

      case EAI_SERVICE:
         return "EAI_SERVICE";

      case EAI_SOCKTYPE:
         return "EAI_SOCKTYPE";

      case EAI_SYSTEM:
         return "EAI_SYSTEM";

      case EAI_OVERFLOW:
         return "EAI_OVERFLOW";

// GNU

      case EAI_NODATA:
         return "EAI_NODATA";

      case EAI_ADDRFAMILY:
         return "EAI_ADDRFAMILY";

      case EAI_INPROGRESS:
         return "EAI_INPROGRESS";

      case EAI_CANCELED:
         return "EAI_CANCELED";

      case EAI_NOTCANCELED:
         return "EAI_NOTCANCELED";

      case EAI_ALLDONE:
         return "EAI_ALLDONE";

      case EAI_INTR:
         return "EAI_INTR";

      case EAI_IDN_ENCODE:
         return "EAI_IDN_ENCODE";

      default:
         std::stringstream str;
         str << "Unknown(netdb.h error code = " << errno << ")";
         return str.str();
   }
}

