///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_NETAPI_H
#define INCLUDED_NETAPI_H

#include "netdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(INetwork);
F_DECLARE_INTERFACE(INetSocket);
F_DECLARE_INTERFACE(INetAddress);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: INetwork
//

interface INetwork : IUnknown
{
};

///////////////////////////////////////

NET_API tResult NetworkCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: INetSocket
//

interface INetSocket : IUnknown
{
   virtual tResult Bind(INetAddress * pAddress) = 0;

   virtual tResult Connect(INetAddress * pAddress) = 0;

   virtual tResult Listen(int maxConnections) = 0;

   virtual tResult Send(const void * pBuffer, int nBufferBytes, int * pnBytesSent) = 0;
   virtual tResult SendTo(const void * pBuffer, int nBufferBytes, INetAddress * pAddress, int * pnBytesSent) = 0;

   virtual tResult Receive(void * pBuffer, int nBufferBytes, int * pnBytesReceived) = 0;
   virtual tResult ReceiveFrom(void * pBuffer, int nBufferBytes, int * pnBytesReceived, INetAddress * * ppAddress) = 0;

   virtual tResult Close() = 0;
};

////////////////////////////////////////

NET_API tResult NetSocketCreateDatagram(INetSocket * * ppSocket);
NET_API tResult NetSocketCreateStream(INetSocket * * ppSocket);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: INetAddress
//

interface INetAddress : IUnknown
{
   virtual const void * GetAddress() const = 0;
   virtual int GetAddressLength() const = 0;
};

////////////////////////////////////////

NET_API tResult NetAddressCreate(const void * pAddr, int addrLength, INetAddress * * ppAddress);
NET_API tResult NetAddressCreateIPv4(const char * pszAddress, uint16 port, INetAddress * * ppAddress);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_NETAPI_H
