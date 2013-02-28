///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RESOURCEMANAGER_H
#define INCLUDED_RESOURCEMANAGER_H

#include "resourceformat.h"
#include "resourceutils.h"

#include "tech/resourceapi.h"
#include "tech/globalobjdef.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IResourceStore);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

class cResourceManager : public cComObject3<IMPLEMENTS(IResourceManager),
                                            IMPLEMENTS(IGlobalObject),
                                            IMPLEMENTS(IResourceManagerDiagnostics)>
{
   friend class cResourceManagerTests;

   typedef std::map<cResourceCacheKey, cResourceData> tResourceCache;

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
   virtual tResult Load(const tChar * pszName, tResourceType type, void * loadParam, void * * ppData);
   tResult LoadWithFormat(const tChar * pszName, tResourceType type, uint formatId, void * param, void * * ppData);
   virtual tResult Unload(const tChar * pszName, tResourceType type);
   tResult Unload(tResourceCache::iterator iter);
   void UnloadAll();
   virtual tResult RegisterFormat(tResourceType type,
                                  tResourceType typeDepend,
                                  const tChar * pszExtension,
                                  tResourceLoad pfnLoad,
                                  tResourcePostload pfnPostload,
                                  tResourceUnload pfnUnload,
                                  void * typeParam);
   virtual tResult ListResources(const tChar * pszMatch, std::vector<cStr> * pNames) const;

   // IResourceManagerDiagnostics
   virtual void DumpFormats() const;
   virtual void DumpCache() const;
   virtual size_t GetCacheSize() const;

private:
   tResult Open(const tChar * pszName, IReader * * ppReader);
   tResult DoLoadFromReader(IReader * pReader, const cResourceFormat * pFormat, ulong dataSize, void * param, void * * ppData);

   typedef std::vector<IResourceStore *> tResourceStores;
   tResourceStores m_stores;

   cResourceFormatTable m_formats;

   tResourceCache m_cache;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RESOURCEMANAGER_H
