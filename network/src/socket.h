///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SOCKET_H
#define INCLUDED_SOCKET_H

#ifdef _MSC_VER
#pragma once
#endif

struct sockaddr;

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

   int ReceiveFrom(void * pBuffer, int nBufferBytes, struct sockaddr * pAddr, int * pAddrLength);

private:
   uint m_socket;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SOCKET_H
