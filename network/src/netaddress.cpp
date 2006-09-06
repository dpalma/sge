///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "netaddress.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetAddress
//

///////////////////////////////////////

cNetAddress::cNetAddress()
 : m_pAddr(NULL)
 , m_addrLength(0)
{
}

///////////////////////////////////////

cNetAddress::cNetAddress(const void * pAddr, int addrLength)
 : m_pAddr(NULL)
 , m_addrLength(0)
{
   if (pAddr != NULL && addrLength > 0)
   {
      m_pAddr = malloc(addrLength);
      if (m_pAddr != NULL)
      {
         memcpy(m_pAddr, pAddr, sizeof(addrLength));
      }
   }
}

///////////////////////////////////////

cNetAddress::~cNetAddress()
{
   if (m_pAddr != NULL)
   {
      free(m_pAddr);
      m_pAddr = NULL;
      m_addrLength = 0;
   }
}

///////////////////////////////////////

const void * cNetAddress::GetAddress() const
{
   return m_pAddr;
}

///////////////////////////////////////

int cNetAddress::GetAddressLength() const
{
   return m_addrLength;
}

///////////////////////////////////////////////////////////////////////////////

tResult NetAddressCreate(const void * pAddr, int addrLength, INetAddress * * ppAddress)
{
   if (pAddr == NULL || ppAddress == NULL)
   {
      return E_POINTER;
   }

   if (addrLength <= 0)
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<INetAddress> pAddress(static_cast<INetAddress*>(new cNetAddress(pAddr, addrLength)));
   if (!pAddress)
   {
      return E_OUTOFMEMORY;
   }

   if (pAddress->GetAddress() == NULL || pAddress->GetAddressLength() != addrLength)
   {
      return E_FAIL;
   }

   return pAddress.GetPointer(ppAddress);
}


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

int cNetAddressIPv4::GetAddressLength() const
{
   return sizeof(m_addr);
}

///////////////////////////////////////////////////////////////////////////////

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

#ifdef HAVE_UNITTESTPP

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
