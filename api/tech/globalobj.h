///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GLOBALOBJ_H
#define INCLUDED_GLOBALOBJ_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGlobalObjectRegistry
//

interface IGlobalObjectRegistry : IUnknown
{
   virtual tResult Register(REFGUID iid, IUnknown * pUnk) = 0;
   virtual IUnknown * Lookup(REFGUID iid) = 0;

   virtual tResult InitAll() = 0;
   virtual void TermAll() = 0;
};

///////////////////////////////////////////////////////////////////////////////

tResult StartGlobalObjects();
void StopGlobalObjects();

extern TECH_API IGlobalObjectRegistry * g_pGlobalObjectRegistry;

inline IUnknown * FindGlobalObject(REFGUID iid)
{
   return g_pGlobalObjectRegistry->Lookup(iid);
}

#define UseGlobal_(ObjBaseName, VarName) \
   cAutoIPtr<I##ObjBaseName> VarName(static_cast<I##ObjBaseName*>(FindGlobalObject(IID_I##ObjBaseName)))

#define UseGlobal(ObjBaseName) \
   UseGlobal_(ObjBaseName, p##ObjBaseName)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLOBALOBJ_H
