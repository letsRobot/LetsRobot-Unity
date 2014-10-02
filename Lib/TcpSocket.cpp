#include "TcpSocket.h"

void TcpThrow(TcpSocketException::Type type, int errorCode)
{
   throw TcpSocketException(type, TcpSocketException::None, errorCode);
}

void TcpThrow(TcpSocketException::Type type, TcpSocketException::Reason reason, int errorCode)
{
   throw TcpSocketException(type, reason, errorCode);
}
