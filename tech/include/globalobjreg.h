///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJREG_H
#define INCLUDED_GLOBALOBJREG_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGlobalObjectRegistry);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGlobalObjectRegistry
//

interface IGlobalObjectRegistry : IUnknown
{
   virtual tResult Register(REFGUID iid, IUnknown * pUnk) = 0;
   virtual IUnknown * Lookup(REFGUID iid) = 0;

   virtual tResult InitAll() = 0;
   virtual tResult TermAll() = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLOBALOBJREG_H
