///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEMANAGER_H
#define INCLUDED_RESOURCEMANAGER_H

#include "resourceapi.h"
#include "globalobjdef.h"
#include "resourceformat.h"
#include "resourcestore.h"

#ifdef _MSC_VER
#pragma once
#endif

class cFileSpec;

F_DECLARE_INTERFACE(IResourceManagerDiagnostics);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceCacheKey
//

class cResourceCacheKey
{
public:
   cResourceCacheKey(const tChar * pszName);
   cResourceCacheKey(const cResourceCacheKey &);
   virtual ~cResourceCacheKey();

   const cResourceCacheKey & operator =(const cResourceCacheKey &);

   inline const tChar * GetName() const { return m_name.c_str(); }

private:
   cStr m_name;
};


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sResource
//

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
                                            IMPLEMENTS(IResourceManagerDiagnostics)>
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
   virtual tResult ListResources(const tChar * pszMatch, std::vector<cStr> * pNames) const;

   // IResourceManagerDiagnostics
   virtual void DumpFormats() const;
   virtual void DumpCache() const;
   virtual size_t GetCacheSize() const;

private:
   sResource * FindResourceWithFormat(const tChar * pszName, tResourceType type, uint formatId);
   tResult DoLoadFromReader(IReader * pReader, const cResourceFormat * pFormat, ulong dataSize, void * param, void * * ppData);

   std::vector<cResourceStore *> m_stores;

   cResourceFormatTable m_formats;

   typedef std::vector<sResource> tResources;
   tResources m_resources;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEMANAGER_H
