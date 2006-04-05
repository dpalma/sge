///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "netsocket.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <ctime>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetSocket
//

///////////////////////////////////////

cNetSocket::cNetSocket()
 : m_socket(INVALID_SOCKET)
{
}

///////////////////////////////////////

cNetSocket::~cNetSocket()
{
}

///////////////////////////////////////

bool cNetSocket::Create(uint port, uint type, const char * pszAddress)
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

int cNetSocket::Receive(void * pBuffer, int nBufferBytes)
{
   if (m_socket != INVALID_SOCKET)
   {
      return recv(m_socket, (char *)pBuffer, nBufferBytes, 0);
   }
   return SOCKET_ERROR;
}

///////////////////////////////////////

int cNetSocket::ReceiveFrom(void * pBuffer, int nBufferBytes, struct sockaddr * pAddr, int * pAddrLength)
{
   if (m_socket != INVALID_SOCKET)
   {
      return recvfrom(m_socket, (char *)pBuffer, nBufferBytes, 0, pAddr, pAddrLength);
   }
   return SOCKET_ERROR;
}

///////////////////////////////////////

int cNetSocket::SendTo(const void * pBuffer, int nBufferBytes, const sockaddr * pAddr, int addrLen, int flags)
{
   if (m_socket != INVALID_SOCKET)
   {
      return sendto(m_socket, (const char *)pBuffer, nBufferBytes, flags, pAddr, addrLen);
   }
   return SOCKET_ERROR;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

////////////////////////////////////////
// Simplest possible datagram send/receive test

/*
TEST(SimpleDatagramSendRecieve)
{
   static const int kBufferSize = 100;
   static const int kPort = 1010;

   srand(time(NULL));

   // Create send socket

   cNetSocket sendSocket;
   CHECK(sendSocket.Create(0, SOCK_DGRAM, NULL));

   // Create receive socket

   cNetSocket receiveSocket;
   CHECK(receiveSocket.Create(kPort, SOCK_DGRAM, NULL));

   // Send

   char sendBuffer[kBufferSize];
   for (int i = 0; i < kBufferSize; i++)
   {
      sendBuffer[i] = rand() & 0xFF;
   }

   cNetAddress sendAddress("127.0.0.1", kPort);
   CHECK(sendAddress.GetSockAddr() != NULL);

   CHECK(sendSocket.SendTo(sendBuffer, sizeof(sendBuffer),
      sendAddress.GetSockAddr(), sendAddress.GetSockAddrSize()) == sizeof(sendBuffer));

   // Receive

   char recvBuffer[kBufferSize];
   memset(recvBuffer, 0, sizeof(recvBuffer));

   CHECK(receiveSocket.Receive(recvBuffer, sizeof(recvBuffer)) == sizeof(recvBuffer));

   // Verify

   CHECK(memcmp(sendBuffer, recvBuffer, kBufferSize) == 0);
}
*/

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
