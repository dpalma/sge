///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCESTORE_H
#define INCLUDED_RESOURCESTORE_H

#include "tech/comtools.h"
#include "tech/techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IResourceStore);

F_DECLARE_INTERFACE(IReader);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: IResourceStore
//

interface IResourceStore : IUnknown
{
   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames) = 0;
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader) = 0;
};

tResult ResourceStoreCreateZip(const tChar * pszArchive, IResourceStore * * ppStore);

tResult ResourceStoreCreateFileSystem(const tChar * pszDir, IResourceStore * * ppStore);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
