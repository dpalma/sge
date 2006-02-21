///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEMANAGER_H
#define INCLUDED_RESOURCEMANAGER_H

#include "resourceapi.h"
#include "filepath.h"
#include "globalobjdef.h"

#include <map>

#define ZLIB_WINAPI
#include <unzip.h>

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

const uint kNoIndex = ~0;
const ulong kNoIndexL = ~0;

F_DECLARE_INTERFACE(IResourceManagerDiagnostics);

class cResourceStore;
class cResourceCache;
class cResourceCacheEntryHeader;


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManagerDiagnostics
//

interface IResourceManagerDiagnostics : IUnknown
{
   virtual void DumpFormats() const = 0;
   virtual void DumpCache() const = 0;
};


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
//
// CLASS: cResourceManager
//

class cResourceManager : public cComObject3<IMPLEMENTS(IResourceManager),
                                            IMPLEMENTS(IGlobalObject),
                                            IMPLEMENTS(IResourceManagerDiagnostics)>,
                         public cResourceCache
{
   friend class cResourceManagerTests;

public:
   cResourceManager();
   virtual ~cResourceManager();

   DECLARE_NAME_STRING(kResourceManagerName)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   // IResourceManager methods
   virtual tResult AddDirectory(const tChar * pszDir);
   virtual tResult AddDirectoryTreeFlattened(const tChar * pszDir);
   virtual tResult AddArchive(const tChar * pszArchive);
   virtual tResult Load(const tChar * pszName, tResourceType type, void * param, void * * ppData);
   tResult LoadWithFormat(const tChar * pszName, tResourceType type, uint formatId, void * param, void * * ppData);
   virtual tResult Unload(const tChar * pszName, tResourceType type);
   virtual tResult RegisterFormat(tResourceType type,
                                  tResourceType typeDepend,
                                  const tChar * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload);
   virtual tResult ListResources(tResourceType type, std::vector<cStr> * pNames) const;

   virtual tResult AddCacheEntry(const cResourceCacheEntryHeader & entry);

   virtual void DumpFormats() const;
   virtual void DumpCache() const;

private:
   struct sFormat;
   struct sResource;

   sResource * FindResourceWithFormat(const tChar * pszName, tResourceType type, uint formatId);
   tResult DoLoadFromFile(const cFileSpec & file, const sFormat * pFormat, void * param, ulong * pDataSize, void * * ppData);
   tResult DoLoadFromArchive(uint archiveId, ulong offset, ulong index, const sFormat * pFormat, void * param, ulong * pDataSize, void * * ppData);
   tResult DoLoadFromReader(IReader * pReader, const sFormat * pFormat, ulong dataSize, void * param, void * * ppData);

   uint DeduceFormats(const tChar * pszName, tResourceType type, uint * pFormatIds, uint nMaxFormats);

   uint GetExtensionId(const tChar * pszExtension);
   uint GetExtensionIdForName(const tChar * pszName);
   uint GetDirectoryId(const tChar * pszDir);
   uint GetArchiveId(const tChar * pszArchive);

   std::vector<cStr> m_extensions;
   std::vector<cFilePath> m_dirs;

   std::vector<cResourceStore *> m_stores;

   struct sArchiveInfo
   {
      cStr archive;
      unzFile handle;
   };
   typedef std::vector<sArchiveInfo> tArchives;
   tArchives m_archives;

   struct sFormat
   {
      tResourceType type;
      tResourceType typeDepend; // if not NULL, this format loads 'type' by converting from 'typeDepend'
      uint extensionId;
      tResourceLoad pfnLoad;
      tResourcePostload pfnPostload;
      tResourceUnload pfnUnload;
   };
   typedef std::vector<sFormat> tFormats;
   tFormats m_formats;

   typedef std::multimap<cStr, uint> tExtsForType;
   tExtsForType m_extsForType;

   struct sResource
   {
      sResource();
      sResource(const sResource &);
      ~sResource();

      const sResource & operator =(const sResource &);

      cStr name;
      uint extensionId;
      uint formatId;
      uint dirId;
      uint archiveId;
      cResourceStore * pStore;
      ulong offset;
      ulong index;
      void * pData;
      ulong dataSize;
   };
   typedef std::vector<sResource> tResources;
   tResources m_resources;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEMANAGER_H
