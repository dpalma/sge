///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "socket.h"

#include <winsock.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSocket
//

///////////////////////////////////////

cSocket::cSocket()
 : m_socket(INVALID_SOCKET)
{
}

///////////////////////////////////////

cSocket::~cSocket()
{
}

///////////////////////////////////////

bool cSocket::Init()
{
   WSADATA wsaData;
   return (WSAStartup(MAKEWORD(1,1), &wsaData) == 0);
}

///////////////////////////////////////

void cSocket::Term()
{
   WSACleanup();
}

///////////////////////////////////////

bool cSocket::Create(uint port, uint type, const char * pszAddress)
{
   if (m_socket == INVALID_SOCKET)
   {
      m_socket = socket(AF_INET, type, 0);
      if (m_socket != INVALID_SOCKET)
      {
         struct sockaddr_in addr;
         addr.sin_family = AF_INET;
         addr.sin_addr.s_addr = htonl(INADDR_ANY);
         addr.sin_port = htons(port);
         if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR)
         {
            return true;
         }
      }
   }
   return false;
}

///////////////////////////////////////

int cSocket::ReceiveFrom(void * pBuffer, int nBufferBytes, struct sockaddr * pAddr, int * pAddrLength)
{
   if (m_socket != INVALID_SOCKET)
   {
      return recvfrom(m_socket, (char *)pBuffer, nBufferBytes, 0, pAddr, pAddrLength);
   }
   return SOCKET_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
