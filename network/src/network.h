///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_NETWORK_H
#define INCLUDED_NETWORK_H

#include "netapi.h"

#include "globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNetwork
//

class cNetwork : public cComObject2<IMPLEMENTS(INetwork), IMPLEMENTS(IGlobalObject)>
{
public:
   cNetwork();
   virtual ~cNetwork();

   DECLARE_NAME(Network)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_NETWORK_H
