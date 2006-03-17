///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEMANAGER_H
#define INCLUDED_RESOURCEMANAGER_H

#include "resourceapi.h"
#include "globalobjdef.h"
#include "resourcestore.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

F_DECLARE_INTERFACE(IResourceManagerDiagnostics);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IResourceManagerDiagnostics
//

interface IResourceManagerDiagnostics : IUnknown
{
   virtual void DumpFormats() const = 0;
   virtual void DumpCache() const = 0;
   virtual size_t GetCacheSize() const = 0;
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
   tResult AddResourceStore(cResourceStore * pStore);
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

   // IResourceManagerDiagnostics
   virtual void DumpFormats() const;
   virtual void DumpCache() const;
   virtual size_t GetCacheSize() const;

private:
   struct sFormat;
   struct sResource;

   sResource * FindResourceWithFormat(const tChar * pszName, tResourceType type, uint formatId);
   tResult DoLoadFromReader(IReader * pReader, const sFormat * pFormat, ulong dataSize, void * param, void * * ppData);

   uint DeduceFormats(const tChar * pszName, tResourceType type, uint * pFormatIds, uint nMaxFormats);

   uint GetExtensionId(const tChar * pszExtension);
   uint GetExtensionIdForName(const tChar * pszName);

   std::vector<cStr> m_extensions;

   std::vector<cResourceStore *> m_stores;

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
      cResourceStore * pStore;
      void * pData;
      ulong dataSize;
   };
   typedef std::vector<sResource> tResources;
   tResources m_resources;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEMANAGER_H
