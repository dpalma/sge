///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESMGR_H
#define INCLUDED_RESMGR_H

#include "techdll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);
F_DECLARE_INTERFACE(IResourceManager);

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManager
//

interface IResourceManager : IUnknown
{
   virtual IReader * Find(const char * pszName) = 0;

   virtual void AddSearchPath(const char * pszPath) = 0;
};

#define kResourceManagerName "ResourceManager"

///////////////////////////////////////

TECH_API void ResourceManagerCreate();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESMGR_H
