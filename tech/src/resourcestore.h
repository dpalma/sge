///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCESTORE_H
#define INCLUDED_RESOURCESTORE_H

#include "resourceapi.h"

#define ZLIB_WINAPI
#include <unzip.h>

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
   virtual tResult OpenEntry(const cResourceCacheEntryHeader & entry, IReader * * ppReader) = 0;
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
   virtual tResult OpenEntry(const cResourceCacheEntryHeader & entry, IReader * * ppReader);

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
   virtual tResult OpenEntry(const cResourceCacheEntryHeader & entry, IReader * * ppReader);

private:
   cStr m_archive;
   unzFile m_handle;
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
   cResourceCacheEntryHeader(const tChar * pszName, ulong offset, ulong index, cResourceStore * pStore);
   cResourceCacheEntryHeader(const cResourceCacheEntryHeader &);
   virtual ~cResourceCacheEntryHeader();

   const cResourceCacheEntryHeader & operator =(const cResourceCacheEntryHeader &);

   //bool operator ==(const cResourceCacheEntryHeader &) const;

   inline const tChar * GetName() const { return m_name.c_str(); }
   inline ulong GetOffset() const { return m_offset; }
   inline ulong GetIndex() const { return m_index; }
   inline cResourceStore * GetStore() const { return m_pStore; }

private:
   cStr m_name;
   ulong m_offset;
   ulong m_index;
   cResourceStore * m_pStore;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
