///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOCKET_H
#define INCLUDED_SOCKET_H

#include "str.h"

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
// CLASS: cNetAddress
//

class cNetAddress
{
public:
   cNetAddress();
   cNetAddress(const char * pszAddress, int port);
   ~cNetAddress();

   const struct sockaddr * GetSockAddr() const;
   size_t GetSockAddrSize() const;

private:
   struct sSockAddrIn
   {
      short family;
      ushort port;
      long addr;
      char zero[8];
   };

   cStr m_address;
   int m_port;
   mutable sSockAddrIn m_sockAddrIn;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSocket
//

class cSocket
{
public:
   cSocket();
   virtual ~cSocket();

   static bool Init();
   static void Term();

   bool Create(uint port, uint type, const char * pszAddress);

   int Receive(void * pBuffer, int nBufferBytes);
   int ReceiveFrom(void * pBuffer, int nBufferBytes, struct sockaddr * pAddr, int * pAddrLength);

	int SendTo(const void * pBuffer, int nBufferBytes, const sockaddr * pAddr, int addrLen, int flags = 0);
	int SendTo(const void * pBuffer, int nBufferBytes, const cNetAddress & address, int flags = 0);

private:
   uint m_socket;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOCKET_H
