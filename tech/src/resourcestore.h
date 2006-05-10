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


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceStore
//

class cResourceStore
{
public:
   virtual ~cResourceStore() = 0;

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames) = 0;
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

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
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

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   cStr m_archive;
   unzFile m_handle;
   typedef std::map<cStr, unz_file_pos_s> tZipDirCache;
   tZipDirCache m_dirCache;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCESTORE_H
