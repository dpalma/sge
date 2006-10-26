///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "network.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetwork
//

tResult NetworkCreate()
{
   cAutoIPtr<INetwork> p(static_cast<INetwork*>(new cNetwork));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_INetwork, p);
}

///////////////////////////////////////

cNetwork::cNetwork()
{
}

///////////////////////////////////////

cNetwork::~cNetwork()
{
}

///////////////////////////////////////

tResult cNetwork::Init()
{
#ifdef _WIN32
   WSADATA wsaData = {0};
   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
   {
      return E_FAIL;
   }
#endif
   return S_OK;
}

///////////////////////////////////////

tResult cNetwork::Term()
{
#ifdef _WIN32
   WSACleanup();
#endif
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
