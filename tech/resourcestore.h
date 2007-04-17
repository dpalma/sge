///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCESTORE_H
#define INCLUDED_RESOURCESTORE_H

#include "tech/resourceapi.h"

#define ZLIB_WINAPI
#include <unzip.h>

#include <map>

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
//
// CLASS: cZipResourceStore
//

class cZipResourceStore : public cComObject<IMPLEMENTS(IResourceStore)>
{
public:
   cZipResourceStore(const tChar * pszArchive);
   virtual ~cZipResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_archive;
   unzFile m_handle;
   typedef std::map<cStr, unz_file_pos_s> tZipDirCache;
   tZipDirCache m_dirCache;
};

////////////////////////////////////////

tResult ZipResourceStoreCreate(const tChar * pszArchive, IResourceStore * * ppStore);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
