///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MEMMANAGER_H
#define INCLUDED_MEMMANAGER_H

#include "techdll.h"
#include "comtools.h"
#include <stddef.h>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IMemManager);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IMemManager
//

interface IMemManager : IUnknown
{
   typedef size_t size_type;
   typedef void * ptr_type;

   virtual void ManageMemory(ptr_type pMem, size_type size) = 0;

   virtual ptr_type GetManagedMemory() const = 0;

   virtual ptr_type Alloc(size_type size) = 0;
   virtual void Free(ptr_type p) = 0;
};

TECH_API IMemManager * MemManagerCreate(void * pMem = NULL, size_t size = 0);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MEMMANAGER_H
