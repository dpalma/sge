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

TECH_API tResult StartGlobalObjects();
TECH_API tResult StopGlobalObjects();

TECH_API IUnknown * FindGlobalObject(REFGUID iid);

#define UseGlobal_(ObjBaseName, VarName) \
   cAutoIPtr<I##ObjBaseName> VarName(static_cast<I##ObjBaseName*>(FindGlobalObject(IID_I##ObjBaseName)))

#define UseGlobal(ObjBaseName) \
   UseGlobal_(ObjBaseName, p##ObjBaseName)

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GLOBALOBJ_H
