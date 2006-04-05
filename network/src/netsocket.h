///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_NETSOCKET_H
#define INCLUDED_NETSOCKET_H

#include "netapi.h"
#include "techstring.h"

#ifdef _MSC_VER
#pragma once
#endif

struct sockaddr;

#ifndef _WIN32
typedef uint SOCKET;
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

class cNetSocket
{
public:
   cNetSocket();
   virtual ~cNetSocket();

   bool Create(uint port, uint type, const char * pszAddress);

   int Receive(void * pBuffer, int nBufferBytes);
   int ReceiveFrom(void * pBuffer, int nBufferBytes, struct sockaddr * pAddr, int * pAddrLength);

	int SendTo(const void * pBuffer, int nBufferBytes, const sockaddr * pAddr, int addrLen, int flags = 0);

private:
   uint m_socket;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_NETSOCKET_H
