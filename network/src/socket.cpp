///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "socket.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#ifdef HAVE_CPPUNIT
#include <time.h>
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetAddress
//

///////////////////////////////////////

cNetAddress::cNetAddress()
 : m_address(""),
   m_port(0)
{
   memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
}

///////////////////////////////////////

cNetAddress::cNetAddress(const char * pszAddress, int port)
 : m_address(pszAddress != NULL ? pszAddress : ""),
   m_port(port)
{
   memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
}

///////////////////////////////////////

cNetAddress::~cNetAddress()
{
}

///////////////////////////////////////

const struct sockaddr * cNetAddress::GetSockAddr() const
{
   if (!m_address.empty())
   {
      struct hostent * h = gethostbyname(m_address.c_str());
      if (h != NULL)
      {
         m_sockAddrIn.family = h->h_addrtype;
         memcpy(&m_sockAddrIn.addr, h->h_addr_list[0], h->h_length);
         m_sockAddrIn.port = htons(m_port);

         return reinterpret_cast<const struct sockaddr *>(&m_sockAddrIn);
      }
   }

   return NULL;
}

///////////////////////////////////////

size_t cNetAddress::GetSockAddrSize() const
{
   return sizeof(m_sockAddrIn);
}


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
#ifdef _WIN32
   WSADATA wsaData;
   return (WSAStartup(MAKEWORD(1,1), &wsaData) == 0);
#else
   return true;
#endif
}

///////////////////////////////////////

void cSocket::Term()
{
#ifdef _WIN32
   WSACleanup();
#endif
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

int cSocket::Receive(void * pBuffer, int nBufferBytes)
{
   if (m_socket != INVALID_SOCKET)
   {
      return recv(m_socket, (char *)pBuffer, nBufferBytes, 0);
   }
   return SOCKET_ERROR;
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

///////////////////////////////////////

int cSocket::SendTo(const void * pBuffer, int nBufferBytes, const sockaddr * pAddr, int addrLen, int flags)
{
   if (m_socket != INVALID_SOCKET)
   {
      return sendto(m_socket, (const char *)pBuffer, nBufferBytes, flags, pAddr, addrLen);
   }
   return SOCKET_ERROR;
}

///////////////////////////////////////

int cSocket::SendTo(const void * pBuffer, int nBufferBytes, const cNetAddress & address, int flags)
{
   if (m_socket != INVALID_SOCKET)
   {
      return sendto(m_socket, (const char *)pBuffer, nBufferBytes, flags,
         address.GetSockAddr(), address.GetSockAddrSize());
   }
   return SOCKET_ERROR;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cSocketTests : public CppUnit::TestCase
{
   void TestSimpleDatagramSendRecieve();

   CPPUNIT_TEST_SUITE(cSocketTests);
      CPPUNIT_TEST(TestSimpleDatagramSendRecieve);
   CPPUNIT_TEST_SUITE_END();

public:
   virtual void setUp();
   virtual void tearDown();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cSocketTests);

////////////////////////////////////////

void cSocketTests::setUp()
{
   CPPUNIT_ASSERT(cSocket::Init());
   srand(time(NULL));
}

////////////////////////////////////////

void cSocketTests::tearDown()
{
   cSocket::Term();
}

////////////////////////////////////////
// Simplest possible datagram send/receive test

void cSocketTests::TestSimpleDatagramSendRecieve()
{
   static const int kBufferSize = 100;
   static const int kPort = 1010;

   // Create send socket

   cSocket sendSocket;
   CPPUNIT_ASSERT(sendSocket.Create(0, SOCK_DGRAM, NULL));

   // Create receive socket

   cSocket receiveSocket;
   CPPUNIT_ASSERT(receiveSocket.Create(kPort, SOCK_DGRAM, NULL));

   // Send

   char sendBuffer[kBufferSize];
   for (int i = 0; i < kBufferSize; i++)
   {
      sendBuffer[i] = rand() & 0xFF;
   }

   cNetAddress sendAddress("127.0.0.1", kPort);
   CPPUNIT_ASSERT(sendAddress.GetSockAddr() != NULL);

   CPPUNIT_ASSERT(sendSocket.SendTo(sendBuffer, sizeof(sendBuffer),
      sendAddress.GetSockAddr(), sendAddress.GetSockAddrSize()) == sizeof(sendBuffer));

   // Receive

   char recvBuffer[kBufferSize];
   memset(recvBuffer, 0, sizeof(recvBuffer));

   CPPUNIT_ASSERT(receiveSocket.Receive(recvBuffer, sizeof(recvBuffer)) == sizeof(recvBuffer));

   // Verify

   CPPUNIT_ASSERT(memcmp(sendBuffer, recvBuffer, kBufferSize) == 0);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
