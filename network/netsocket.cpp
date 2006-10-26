///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "netsocket.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
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

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetSocket
//

////////////////////////////////////////

cNetSocket::cNetSocket()
 : m_socket(INVALID_SOCKET)
{
}

////////////////////////////////////////

cNetSocket::~cNetSocket()
{
   if (m_socket != INVALID_SOCKET)
   {
      WarnMsg("Closing socket in destructor\n");
      Close();
   }
}

////////////////////////////////////////

tResult cNetSocket::Create(int type)
{
   if (type != SOCK_STREAM && type != SOCK_DGRAM)
   {
      return E_INVALIDARG;
   }

   if (m_socket != INVALID_SOCKET)
   {
      WarnMsg("Close socket before attempting to re-create\n");
      return E_FAIL;
   }

   m_socket = socket(AF_INET, type, 0);

   return (m_socket == INVALID_SOCKET) ? E_FAIL : S_OK;
}

////////////////////////////////////////

tResult cNetSocket::Bind(INetAddress * pAddress)
{
   if (pAddress == NULL)
   {
      return E_POINTER;
   }
   
   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   if (bind(m_socket, static_cast<const struct sockaddr *>(pAddress->GetAddress()),
      pAddress->GetAddressLength()) == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   return false;
}

////////////////////////////////////////

tResult cNetSocket::Connect(INetAddress * pAddress)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cNetSocket::Listen(int maxConnections)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cNetSocket::Send(const void * pBuffer, int nBufferBytes, int * pnBytesSent)
{
   if (pBuffer == NULL)
   {
      return E_POINTER;
   }

   if (nBufferBytes <= 0)
   {
      return E_INVALIDARG;
   }
   
   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   int result = send(m_socket, static_cast<const char *>(pBuffer), nBufferBytes, 0);

   if (result == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   if (pnBytesSent != NULL)
   {
      *pnBytesSent = result;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cNetSocket::SendTo(const void * pBuffer, int nBufferBytes, INetAddress * pAddress, int * pnBytesSent)
{
   if (pBuffer == NULL || pAddress == NULL)
   {
      return E_POINTER;
   }

   if (nBufferBytes <= 0)
   {
      return E_INVALIDARG;
   }
   
   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   int result = sendto(m_socket, static_cast<const char *>(pBuffer), nBufferBytes, 0,
      static_cast<const struct sockaddr *>(pAddress->GetAddress()),
      pAddress->GetAddressLength());

   if (result == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   if (pnBytesSent != NULL)
   {
      *pnBytesSent = result;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cNetSocket::Receive(void * pBuffer, int nBufferBytes, int * pnBytesReceived)
{
   if (pBuffer == NULL)
   {
      return E_POINTER;
   }
   
   if (nBufferBytes <= 0)
   {
      return E_INVALIDARG;
   }

   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   int result = recv(m_socket, static_cast<char *>(pBuffer), nBufferBytes, 0);

   if (result == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   if (pnBytesReceived != NULL)
   {
      *pnBytesReceived = result;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cNetSocket::ReceiveFrom(void * pBuffer, int nBufferBytes, int * pnBytesReceived, INetAddress * * ppAddress)
{
   if (pBuffer == NULL)
   {
      return E_POINTER;
   }
   
   if (nBufferBytes <= 0)
   {
      return E_INVALIDARG;
   }

   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   struct sockaddr addr = {0};
   int addrLength = sizeof(addr);

   int result = recvfrom(m_socket, static_cast<char *>(pBuffer), nBufferBytes, 0, &addr, &addrLength);

   if (result == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   if (pnBytesReceived != NULL)
   {
      *pnBytesReceived = result;
   }

   if (ppAddress != NULL)
   {
      NetAddressCreate(&addr, addrLength, ppAddress);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cNetSocket::Close()
{
   if (m_socket == INVALID_SOCKET)
   {
      return E_FAIL;
   }

   if (closesocket(m_socket) == SOCKET_ERROR)
   {
      return E_FAIL;
   }

   m_socket = INVALID_SOCKET;
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

tResult NetSocketCreateDatagram(INetSocket * * ppSocket)
{
   if (ppSocket == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cNetSocket> pSocket(new cNetSocket);
   if (!pSocket)
   {
      return E_OUTOFMEMORY;
   }

   if (pSocket->Create(SOCK_DGRAM) != S_OK)
   {
      return E_FAIL;
   }

   *ppSocket = CTAddRef(static_cast<INetSocket*>(pSocket));
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult NetSocketCreateStream(INetSocket * * ppSocket)
{
   if (ppSocket == NULL)
   {
      return E_POINTER;
   }

   return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////
// Simplest possible datagram send/receive test

#if 0
TEST(SimpleDatagramSendRecieve)
{
   // Create send socket

   cAutoIPtr<INetSocket> pSendSocket;
   CHECK(NetSocketCreateDatagram(&pSendSocket) == S_OK);

   // Create receive socket

   cAutoIPtr<INetSocket> pReceiveSocket;
   CHECK(NetSocketCreateDatagram(&pReceiveSocket) == S_OK);

   static const int kPort = 1010;
   cAutoIPtr<INetAddress> pReceiveAddress;
   CHECK(NetAddressCreateIPv4(NULL, kPort, &pReceiveAddress) == S_OK);
   CHECK(pReceiveSocket->Bind(pReceiveAddress) == S_OK);

   // Send

   static const int kBufferSize = 100;
   char sendBuffer[kBufferSize];
   srand(time(NULL));
   for (int i = 0; i < kBufferSize; i++)
   {
      sendBuffer[i] = rand() & 0xFF;
   }

   cAutoIPtr<INetAddress> pSendAddress;
   CHECK(NetAddressCreateIPv4("127.0.0.1", kPort, &pSendAddress) == S_OK);

   int nBytesSent = 0;
   CHECK(pSendSocket->SendTo(sendBuffer, sizeof(sendBuffer), pSendAddress, &nBytesSent) == S_OK);

   // Receive

   char recvBuffer[kBufferSize];
   memset(recvBuffer, 0, sizeof(recvBuffer));

   int nBytesReceived = 0;
   CHECK(pReceiveSocket->Receive(recvBuffer, sizeof(recvBuffer), &nBytesReceived) == S_OK);
   CHECK_EQUAL(nBytesReceived, sizeof(recvBuffer));

   // Verify

   CHECK(memcmp(sendBuffer, recvBuffer, kBufferSize) == 0);
}
#endif

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
