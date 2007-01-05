///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_NETSOCKET_H
#define INCLUDED_NETSOCKET_H

#include "network/netapi.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _WIN32
typedef uint SOCKET;
#endif

#ifdef _WIN32
typedef int socklen_t;
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetSocket
//

class cNetSocket : public cComObject<IMPLEMENTS(INetSocket)>
{
public:
   cNetSocket();
   virtual ~cNetSocket();

   tResult Create(int type);

   virtual tResult Bind(INetAddress * pAddress);

   virtual tResult Connect(INetAddress * pAddress);

   virtual tResult Listen(int maxConnections);

   virtual tResult Send(const void * pBuffer, int nBufferBytes, int * pnBytesSent);
   virtual tResult SendTo(const void * pBuffer, int nBufferBytes, INetAddress * pAddress, int * pnBytesSent);

   virtual tResult Receive(void * pBuffer, int nBufferBytes, int * pnBytesReceived);
   virtual tResult ReceiveFrom(void * pBuffer, int nBufferBytes, int * pnBytesReceived, INetAddress * * pAddress);

   virtual tResult Close();

private:
   uint m_socket;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_NETSOCKET_H
