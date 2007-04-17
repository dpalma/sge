////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcemanager.h"
#include "resourcestore.h"

#include "tech/fileenum.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/readwriteapi.h"

#include <cstdio>
#include <vector>
#include <algorithm>
#include <set>

#include "tech/dbgalloc.h" // must be last header

using namespace std;

////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ResourceManager);

#define LocalMsg(msg)            DebugMsgEx(ResourceManager,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ResourceManager,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ResourceManager,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ResourceManager,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(ResourceManager,msg,(a),(b),(c),(d))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(ResourceManager,(cond),msg)
#define LocalMsgIf1(cond,msg,a)        DebugMsgIfEx1(ResourceManager,(cond),msg,(a))
#define LocalMsgIf2(cond,msg,a,b)      DebugMsgIfEx2(ResourceManager,(cond),msg,(a),(b))
#define LocalMsgIf3(cond,msg,a,b,c)    DebugMsgIfEx3(ResourceManager,(cond),msg,(a),(b),(c))
#define LocalMsgIf4(cond,msg,a,b,c,d)  DebugMsgIfEx4(ResourceManager,(cond),msg,(a),(b),(c),(d))

////////////////////////////////////////////////////////////////////////////////

// REFERENCES
// "Game Developer Magazine", February 2005, "Inner Product" column


////////////////////////////////////////////////////////////////////////////////

void * ThunkResourceLoadNoParam(IReader * pReader, void * typeParam)
{
   tResourceLoadNoParam pfn = (tResourceLoadNoParam)typeParam;
   if (pfn != NULL)
   {
      return (*pfn)(pReader);
   }
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManager
//

////////////////////////////////////////

cResourceManager::cResourceManager()
{
}

////////////////////////////////////////

cResourceManager::~cResourceManager()
{
}

////////////////////////////////////////

tResult cResourceManager::Init()
{
   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::Term()
{
   UnloadAll();

   {
      for_each(m_stores.begin(), m_stores.end(), CTInterfaceMethod(&IResourceStore::Release));
      m_stores.clear();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectory(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   IResourceStore * pStore = static_cast<IResourceStore *>(new cDirectoryResourceStore(pszDir));
   if (pStore == NULL)
   {
      return E_OUTOFMEMORY;
   }

   LocalMsg1("Adding directory store for \"%s\"\n", pszDir);
   
   m_stores.push_back(pStore);
   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddDirectoryTreeFlattened(const tChar * pszDir)
{
   if (pszDir == NULL)
   {
      return E_POINTER;
   }

   cFilePath root(pszDir);
   root.MakeFullPath();
   if (AddDirectory(root.CStr()) != S_OK)
   {
      return E_FAIL;
   }

   tStrings dirs;
   if (ListDirs(root, true, &dirs) > 0)
   {
      tStrings::const_iterator iter;
      for (iter = dirs.begin(); iter != dirs.end(); iter++)
      {
         if (AddDirectoryTreeFlattened(iter->c_str()) != S_OK)
         {
            return E_FAIL;
         }
      }
   }

   return S_OK;
}

////////////////////////////////////////

tResult cResourceManager::AddArchive(const tChar * pszArchive)
{
   tResult result = E_FAIL;
   IResourceStore * pStore = NULL;
   if ((result = ZipResourceStoreCreate(pszArchive, &pStore)) == S_OK)
   {
      m_stores.push_back(pStore);
      return S_OK;
   }
   return result;
}

////////////////////////////////////////

tResult cResourceManager::Load(const tChar * pszName, tResourceType type,
                               void * loadParam, void * * ppData)
{
   if (pszName == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   if (!type)
   {
      return E_INVALIDARG;
   }

#ifdef _DEBUG
   static bool bDumpCache = false;
   if (bDumpCache)
   {
      DumpCache();
   }
#endif

   tResourceCache::iterator f = m_cache.find(cResourceCacheKey(pszName, type));
   if (f != m_cache.end() && (f->second.GetData() != NULL))
   {
      *ppData = f->second.GetData();
      return S_OK;
   }

   uint formatIds[10];
   uint nFormats = m_formats.DeduceFormats(pszName, type, formatIds, _countof(formatIds));
   for (uint i = 0; i < nFormats; i++)
   {
      if (LoadWithFormat(pszName, type, formatIds[i], loadParam, ppData) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cResourceManager::LoadWithFormat(const tChar * pszName, tResourceType type,
                                         uint formatId, void * loadParam, void * * ppData)
{
   Assert(pszName != NULL);
   Assert(type != NULL);
   Assert(formatId != kNoIndex);
   Assert(ppData != NULL);

   cResourceFormat * pFormat = m_formats.GetFormat(formatId);

#ifdef _DEBUG
   static bool bDumpCache = false;
   if (bDumpCache)
   {
      DumpCache();
   }
#endif

   if (pFormat->typeDepend)
   {
      LocalMsg3("Request for (\"%s\", %s) will be converted from type %s\n",
         pszName, ResourceTypeName(type), ResourceTypeName(pFormat->typeDepend));

      void * pDependData = NULL;
      if (Load(pszName, pFormat->typeDepend, loadParam, &pDependData) == S_OK)
      {
         void * pData = (*pFormat->pfnPostload)(pDependData, 0, loadParam);
         if (pData != NULL)
         {
            // TODO: Store the actual size of the data instead of zero
            m_cache[cResourceCacheKey(pszName, type)] = cResourceData(pData, 0, formatId);
            *ppData = pData;
            return S_OK;
         }
      }
   }
   else
   {
      tResult openResult = E_FAIL;
      cAutoIPtr<IReader> pReader;

      cFileSpec name(pszName);
      if (_tcslen(name.GetFileExt()) > 0)
      {
         openResult = Open(name.CStr(), &pReader);
      }
      else
      {
         set<uint> extsPossible;
         uint extensionId = m_formats.GetExtensionIdForName(pszName);
         if (extensionId == kNoIndex)
         {
            // Deduce possible extensions using the type
            m_formats.GetExtensionsForType(type, &extsPossible);
         }

         set<uint>::const_iterator iter = extsPossible.begin();
         for (; iter != extsPossible.end(); iter++)
         {
            name.SetFileExt(m_formats.GetExtension(*iter));
            openResult = Open(name.CStr(), &pReader);
            if (openResult == S_OK)
            {
               break;
            }
         }
      }

      if ((openResult == S_OK) && !!pReader)
      {
         ulong dataSize = 0;
         void * pData = NULL;
         tResult result = E_FAIL;
         if (pReader->Seek(0, kSO_End) == S_OK
            && pReader->Tell(&dataSize) == S_OK
            && pReader->Seek(0, kSO_Set) == S_OK
            && DoLoadFromReader(pReader, pFormat, dataSize, loadParam, &pData) == S_OK)
         {
            m_cache[cResourceCacheKey(pszName, type)] = cResourceData(pData, dataSize, formatId);
            *ppData = pData;
            result = S_OK;
         }
         return result;
      }
   }

   return E_FAIL;
}

tResult cResourceManager::Open(const tChar * pszName, IReader * * ppReader)
{
   Assert(pszName != NULL);
   Assert(ppReader != NULL);
   tResourceStores::iterator iter = m_stores.begin(), end = m_stores.end();
   for (; iter != end; ++iter)
   {
      cAutoIPtr<IReader> pReader;
      if ((*iter)->OpenEntry(pszName, &pReader) == S_OK)
      {
         return pReader.GetPointer(ppReader);
      }
   }
   return S_FALSE;
}

////////////////////////////////////////

tResult cResourceManager::Unload(const tChar * pszName, tResourceType type)
{
   if (pszName == NULL)
   {
      return E_POINTER;
   }

   if (!type)
   {
      return E_INVALIDARG;
   }

   tResourceCache::iterator f = m_cache.find(cResourceCacheKey(pszName, type));

   if (f == m_cache.end())
   {
      WarnMsg2("Resource (\"%s\", %s) not found\n", pszName, ResourceTypeName(type));
      return S_FALSE;
   }

   tResult result = Unload(f);

   m_cache.erase(f);

   return result;
}

////////////////////////////////////////

tResult cResourceManager::Unload(tResourceCache::iterator iter)
{
   if (iter->second.GetData() != NULL)
   {
      WarnMsgIf(iter->second.GetFormatId() == kNoIndex, "No format id for loaded resource\n");
      if (iter->second.GetFormatId() != kNoIndex)
      {
         cResourceFormat * pFormat = m_formats.GetFormat(iter->second.GetFormatId());
         LocalMsg2("Unloading \"%s\" (%s)\n", iter->first.GetName(), ResourceTypeName(pFormat->type));
         pFormat->Unload(iter->second.GetData());
         return S_OK;
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

void cResourceManager::UnloadAll()
{
   tResourceCache::iterator iter = m_cache.begin(), end = m_cache.end();
   for (; iter != end; ++iter)
   {
      Unload(iter);
   }
   m_cache.clear();
}

////////////////////////////////////////

tResult cResourceManager::RegisterFormat(tResourceType type,
                                         tResourceType typeDepend,
                                         const tChar * pszExtension,
                                         tResourceLoad pfnLoad,
                                         tResourcePostload pfnPostload,
                                         tResourceUnload pfnUnload,
                                         void * typeParam)
{
   return m_formats.RegisterFormat(type, typeDepend, pszExtension, pfnLoad, pfnPostload, pfnUnload, typeParam);
}

////////////////////////////////////////

tResult cResourceManager::ListResources(const tChar * pszMatch, vector<cStr> * pNames) const
{
   if (pszMatch == NULL || pNames == NULL)
   {
      return E_POINTER;
   }

   tResourceStores::const_iterator iter = m_stores.begin(), end = m_stores.end();
   for (; iter != end; ++iter)
   {
      if (FAILED((*iter)->CollectResourceNames(pszMatch, pNames)))
      {
         return E_FAIL;
      }
   }

   unique(pNames->begin(), pNames->end());
   return pNames->empty() ? S_FALSE : S_OK;
}

////////////////////////////////////////

void cResourceManager::DumpFormats() const
{
   m_formats.DumpFormats();
}

////////////////////////////////////////

void cResourceManager::DumpCache() const
{
   LogMsgNoFL1(kInfo, _T("%d resource cache entries\n"), m_cache.size());
   static const int kNameWidth = -30;
   static const int kExtWidth = -5;
   static const int kTypeWidth = -20;
   static const tChar kRowFormat[] = _T("%*s | %*s | %*s\n");
   techlog.Print(NULL, 0, kInfo, kRowFormat,
                 kNameWidth, _T("Name"),
                 kExtWidth, _T("Ext"),
                 kTypeWidth, _T("Type"));
   LogMsgNoFL(kInfo, _T("----------------------------------------------------------------------\n"));
   tResourceCache::const_iterator iter = m_cache.begin();
   for (uint index = 0; iter != m_cache.end(); iter++, index++)
   {
      const cResourceFormat * pFormat = (iter->second.GetFormatId() != kNoIndex)
         ? m_formats.GetFormat(iter->second.GetFormatId()) : NULL;
      techlog.Print(NULL, 0, kInfo, kRowFormat,
         kNameWidth, iter->first.GetName(),
         kExtWidth, _T("None"),
         kTypeWidth, pFormat ? ResourceTypeName(pFormat->type) : _T("Undetermined"));
   }
}

////////////////////////////////////////

size_t cResourceManager::GetCacheSize() const
{
   return m_cache.size();
}

////////////////////////////////////////

void DumpLoadedResources()
{
   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   UseGlobal(ResourceManager);
   if (pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&pResMgrDiag) == S_OK)
   {
      pResMgrDiag->DumpCache();
   }
}

////////////////////////////////////////

void DumpResourceFormats()
{
   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   UseGlobal(ResourceManager);
   if (pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&pResMgrDiag) == S_OK)
   {
      pResMgrDiag->DumpFormats();
   }
}

////////////////////////////////////////

tResult cResourceManager::DoLoadFromReader(IReader * pReader, const cResourceFormat * pFormat, ulong dataSize,
                                           void * loadParam, void * * ppData)
{
   if (pReader == NULL || ppData == NULL)
   {
      return E_POINTER;
   }

   void * pData = pFormat->Load(pReader);
   if (pData != NULL)
   {
      // Assume the postload function cleans up pData or passes
      // it through (or returns NULL)
      pData = pFormat->Postload(pData, dataSize, loadParam);
      if (pData != NULL)
      {
         *ppData = pData;
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult ResourceManagerCreate()
{
   cAutoIPtr<IResourceManager> p(new cResourceManager);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IResourceManager, static_cast<IResourceManager*>(p));
}

////////////////////////////////////////////////////////////////////////////////
