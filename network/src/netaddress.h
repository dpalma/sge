///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_NETADDRESS_H
#define INCLUDED_NETADDRESS_H

#include "netapi.h"
#include "techstring.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#error ("Determine appropriate platform socket headers for addressing")
#endif

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetAddressIPv4
//

class cNetAddressIPv4 : public cComObject<IMPLEMENTS(INetAddress)>
{
public:
   cNetAddressIPv4();
   cNetAddressIPv4(const sockaddr_in & addr);

   ~cNetAddressIPv4();

   virtual const void * GetAddress() const;

private:
   sockaddr_in m_addr;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_NETADDRESS_H
