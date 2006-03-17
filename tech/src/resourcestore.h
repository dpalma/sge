///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCESTORE_H
#define INCLUDED_RESOURCESTORE_H

#include "resourceapi.h"

#define ZLIB_WINAPI
#include <unzip.h>

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

class cResourceStore;
class cResourceCache;
class cResourceCacheEntryHeader;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceStore
//

class cResourceStore
{
public:
   virtual ~cResourceStore() = 0;

   virtual tResult FillCache(cResourceCache * pCache) = 0;
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDirectoryResourceStore
//

class cDirectoryResourceStore : public cResourceStore
{
public:
   cDirectoryResourceStore(const tChar * pszDir);
   virtual ~cDirectoryResourceStore();

   virtual tResult FillCache(cResourceCache * pCache);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_dir;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cZipResourceStore
//

class cZipResourceStore : public cResourceStore
{
public:
   cZipResourceStore(const tChar * pszArchive);
   virtual ~cZipResourceStore();

   virtual tResult FillCache(cResourceCache * pCache);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_archive;
   unzFile m_handle;
   typedef std::map<cStr, unz_file_pos_s> tZipDirCache;
   tZipDirCache m_dirCache;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCache
//

class cResourceCache
{
public:
   virtual ~cResourceCache() = 0;

   virtual tResult AddCacheEntry(const cResourceCacheEntryHeader & entry) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCacheEntryHeader
//

class cResourceCacheEntryHeader
{
public:
   cResourceCacheEntryHeader(const tChar * pszName, cResourceStore * pStore);
   cResourceCacheEntryHeader(const cResourceCacheEntryHeader &);
   virtual ~cResourceCacheEntryHeader();

   const cResourceCacheEntryHeader & operator =(const cResourceCacheEntryHeader &);

   inline const tChar * GetName() const { return m_name.c_str(); }
   inline cResourceStore * GetStore() const { return m_pStore; }

private:
   cStr m_name;
   cResourceStore * m_pStore;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
