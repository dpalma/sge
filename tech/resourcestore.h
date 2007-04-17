///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCESTORE_H
#define INCLUDED_RESOURCESTORE_H

#include "tech/resourceapi.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

F_DECLARE_INTERFACE(IResourceStore);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: IResourceStore
//

interface IResourceStore : IUnknown
{
   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames) = 0;
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader) = 0;
};


////////////////////////////////////////

tResult ResourceStoreCreateZip(const tChar * pszArchive, IResourceStore * * ppStore);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDirectoryResourceStore
//

class cDirectoryResourceStore : public cComObject<IMPLEMENTS(IResourceStore)>
{
public:
   cDirectoryResourceStore(const tChar * pszDir);
   virtual ~cDirectoryResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_dir;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
