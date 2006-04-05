///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "netaddress.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetAddressIPv4
//

///////////////////////////////////////

cNetAddressIPv4::cNetAddressIPv4()
{
   memset(&m_addr, 0, sizeof(m_addr));
}

///////////////////////////////////////

cNetAddressIPv4::cNetAddressIPv4(const sockaddr_in & addr)
{
   memcpy(&m_addr, &addr, sizeof(m_addr));
}

///////////////////////////////////////

cNetAddressIPv4::~cNetAddressIPv4()
{
}

///////////////////////////////////////

const void * cNetAddressIPv4::GetAddress() const
{
   return &m_addr;
}

///////////////////////////////////////

tResult NetAddressCreateIPv4(const char * pszAddress, uint16 port, INetAddress * * ppAddress)
{
   if (ppAddress == NULL)
   {
      return E_POINTER;
   }

   sockaddr_in addr = {0};
   addr.sin_port = htons(port);

   if (pszAddress != NULL)
   {
      struct addrinfo * pAddrInfo = NULL;
      if (getaddrinfo(pszAddress, NULL, NULL, &pAddrInfo) != 0)
      {
         return E_FAIL;
      }

      addr.sin_family = pAddrInfo->ai_family;
      memcpy(&addr.sin_addr, &pAddrInfo->ai_addr, sizeof(addr.sin_addr));

      freeaddrinfo(pAddrInfo);
   }
   else
   {
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
   }

   cAutoIPtr<INetAddress> pAddress(static_cast<INetAddress*>(new cNetAddressIPv4(addr)));
   if (!pAddress)
   {
      return E_OUTOFMEMORY;
   }

   return pAddress.GetPointer(ppAddress);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

TEST(IPv4Address)
{
   {
      cAutoIPtr<INetAddress> pAddress;
      CHECK(NetAddressCreateIPv4(NULL, 4000, &pAddress) == S_OK);
   }

   {
      cAutoIPtr<INetAddress> pAddress;
      CHECK(NetAddressCreateIPv4("127.0.0.1", 4001, &pAddress) == S_OK);
   }
}

#endif

///////////////////////////////////////////////////////////////////////////////
