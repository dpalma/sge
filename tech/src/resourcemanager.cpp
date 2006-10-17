////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "resourcemanager.h"
#include "fileenum.h"
#include "filepath.h"
#include "filespec.h"
#include "readwriteapi.h"
#include "resourcestore.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstdio>
#include <vector>
#include <algorithm>
#include <set>

#include "dbgalloc.h" // must be last header

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
   {
      tResourceCache::iterator iter = m_cache.begin();
      for (; iter != m_cache.end(); iter++)
      {
         if (iter->second.GetData() != NULL)
         {
            WarnMsgIf(iter->second.GetFormatId() == kNoIndex, "No format id for loaded resource\n");
            if (iter->second.GetFormatId() != kNoIndex)
            {
               cResourceFormat * pFormat = m_formats.GetFormat(iter->second.GetFormatId());
               LocalMsg2("Unloading \"%s\" (%s)\n", iter->first.GetName(), ResourceTypeName(pFormat->type));
               pFormat->Unload(iter->second.GetData());
            }
         }
      }
      m_cache.clear();
   }

   {
      std::vector<cResourceStore *>::iterator iter = m_stores.begin();
      for (; iter != m_stores.end(); iter++)
      {
         delete *iter;
      }
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

   cResourceStore * pStore = static_cast<cResourceStore *>(new cDirectoryResourceStore(pszDir));
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
   cResourceStore * pStore = NULL;
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
         std::set<uint> extsPossible;
         uint extensionId = m_formats.GetExtensionIdForName(pszName);
         if (extensionId == kNoIndex)
         {
            // Deduce possible extensions using the type
            m_formats.GetExtensionsForType(type, &extsPossible);
         }

         std::set<uint>::const_iterator iter = extsPossible.begin();
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
   std::vector<cResourceStore *>::iterator iter = m_stores.begin();
   for (; iter != m_stores.end(); iter++)
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
   // TODO: For now resources unloaded only on exit
   return E_NOTIMPL;
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

tResult cResourceManager::ListResources(const tChar * pszMatch, std::vector<cStr> * pNames) const
{
   if (pszMatch == NULL || pNames == NULL)
   {
      return E_POINTER;
   }

   std::vector<cResourceStore *>::const_iterator iter = m_stores.begin();
   for (; iter != m_stores.end(); iter++)
   {
      if (FAILED((*iter)->CollectResourceNames(pszMatch, pNames)))
      {
         return E_FAIL;
      }
   }

   std::unique(pNames->begin(), pNames->end());
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


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

typedef std::pair<cStr, cStr> tStrPair;

class cTestResourceStore : public cResourceStore
{
public:
   cTestResourceStore(const tStrPair * pTestData, size_t nTestData);
   virtual ~cTestResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   // Pairs of <file name, pseudo data>
   std::vector<tStrPair> m_testData;
};

cTestResourceStore::cTestResourceStore(const tStrPair * pTestData, size_t nTestData)
 : m_testData(nTestData)
{
   for (size_t i = 0; i < nTestData; i++, pTestData++)
   {
      m_testData[i] = *pTestData;
   }
}

cTestResourceStore::~cTestResourceStore()
{
}

tResult cTestResourceStore::CollectResourceNames(const tChar * pszMatch, std::vector<cStr> * pNames)
{
   std::vector<std::pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
   for (; iter != m_testData.end(); iter++)
   {
      if (WildCardMatch(pszMatch, iter->first.c_str()))
      {
         pNames->push_back(iter->first.c_str());
      }
   }
   return S_OK;
}

tResult cTestResourceStore::OpenEntry(const tChar * pszName, IReader * * ppReader)
{
   std::vector<std::pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
   for (ulong index = 0; iter != m_testData.end(); iter++, index++)
   {
      if (_tcsicmp(pszName, m_testData[index].first.c_str()) == 0)
      {
         cStr * pDataStr = &m_testData[index].second;
         return MemReaderCreate(reinterpret_cast<const byte *>(pDataStr->c_str()), pDataStr->length(), false, ppReader);
      }
   }
   return E_FAIL;
}

////////////////////////////////////////

class cResourceManagerTests
{
public:
   cResourceManagerTests();
   ~cResourceManagerTests();

   void AddTestData(const tStrPair * pTestData, size_t nTestData);

   IResourceManager * AccessResourceManager() { return static_cast<IResourceManager *>(m_pResourceManager); }
   const IResourceManager * AccessResourceManager() const { return static_cast<const IResourceManager *>(m_pResourceManager); }

   cAutoIPtr<cResourceManager> m_pResourceManager;
   cAutoIPtr<IResourceManagerDiagnostics> m_pDiagnostics;
};

////////////////////////////////////////

cResourceManagerTests::cResourceManagerTests()
{
   SafeRelease(m_pResourceManager);
   m_pResourceManager = new cResourceManager;
   m_pResourceManager->Init();

   cAutoIPtr<IResourceManagerDiagnostics> pResMgrDiag;
   Verify(m_pResourceManager->QueryInterface(IID_IResourceManagerDiagnostics, (void**)&m_pDiagnostics) == S_OK);
}

////////////////////////////////////////

cResourceManagerTests::~cResourceManagerTests()
{
   if (!!m_pResourceManager)
   {
      m_pResourceManager->Term();
      SafeRelease(m_pResourceManager);
   }
}

////////////////////////////////////////

void cResourceManagerTests::AddTestData(const tStrPair * pTestData, size_t nTestData)
{
   if ((pTestData != NULL) && (nTestData > 0))
   {
      cTestResourceStore * pStore = new cTestResourceStore(pTestData, nTestData);
      m_pResourceManager->m_stores.push_back(static_cast<cResourceStore*>(pStore));
   }
}

////////////////////////////////////////

void * RawBytesLoad(IReader * pReader)
{
   ulong dataSize = 0;
   if (pReader != NULL
      && pReader->Seek(0, kSO_End) == S_OK
      && pReader->Tell(&dataSize) == S_OK
      && pReader->Seek(0, kSO_Set) == S_OK)
   {
      byte * pData = new byte[dataSize];
      if (pData != NULL)
      {
         if (pReader->Read(pData, dataSize) != S_OK)
         {
            delete [] pData;
            return NULL;
         }
         else
         {
            return pData;
         }
      }
   }
   return NULL;
}

void RawBytesUnload(void * pData)
{
   delete [] (byte *)pData;
}

// Pass through only if the data starts with an XML declaration
void * PseudoXmlPostload(void * pData, int dataLength, void * loadParam)
{
   const char szXmlDecl[] = "<?xml\0";
   if (memcmp(static_cast<char*>(pData), szXmlDecl, min(static_cast<size_t>(dataLength), strlen(szXmlDecl))) == 0)
   {
      return pData;
   }
   return NULL;
}

void PseudoXmlUnload(void * pData)
{
   // Do nothing
}

void * PseudoMs3dLoad(IReader * pReader)
{
   byte header[10];
   size_t nBytesRead = 0;
   if (pReader->Read(header, sizeof(header), &nBytesRead) == S_OK)
   {
      const byte ms3dHeader[] = "MS3D0000";
      if (memcmp(header, ms3dHeader, min(nBytesRead, _countof(ms3dHeader))) == 0)
      {
         return RawBytesLoad(pReader);
      }
   }
   return NULL;
}

void * PassthruPostload(void * pData, int dataLength, void * loadParam)
{
   return pData;
}

void PassthruUnload(void * pData)
{
   // Do nothing
}

void * ReversePostload(void * pData, int dataLength, void * loadParam)
{
   if (dataLength == 0)
   {
      dataLength = strlen(static_cast<char*>(pData));
   }
   byte * pBytes = static_cast<byte*>(pData);
   for (int i = 0; i < (dataLength / 2); i++)
   {
      byte temp = pBytes[i];
      pBytes[i] = pBytes[dataLength - i - 1];
      pBytes[dataLength - i - 1] = temp;
   }
   return pBytes;
}

////////////////////////////////////////

#define kRT_Data _T("data")
#define kRT_ReverseData _T("RevData")
#define kRT_Bitmap _T("bitmap")

const tStrPair g_basicTestResources[] =
{
   std::make_pair(cStr("foo.dat"), cStr("foo_dat_foo_dat_foo_dat_foo_dat\0")),
   std::make_pair(cStr("foo.bmp"), cStr("foo_bmp_foo_bmp_foo_bmp_foo_bmp\0")),
   std::make_pair(cStr("bar.dat"), cStr("bar_dat_bar_dat_bar_dat_bar_dat\0")),
};

////////////////////////////////////////

TEST_FIXTURE(cResourceManagerTests, ResourceManagerLoadSameNameDifferentExtension)
{
   AddTestData(&g_basicTestResources[0], _countof(g_basicTestResources));

   CHECK(AccessResourceManager()->RegisterFormat(kRT_Data, NULL, "dat", RawBytesLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat(kRT_Bitmap, NULL, "bmp", RawBytesLoad, NULL, RawBytesUnload) == S_OK);

   CHECK_EQUAL(0, m_pDiagnostics->GetCacheSize());

   {
      byte * pFooDat = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat) == S_OK);
      CHECK(memcmp(pFooDat, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   {
      byte * pFooBmp = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Bitmap, (void*)NULL, (void**)&pFooBmp) == S_OK);
      CHECK(memcmp(pFooBmp, g_basicTestResources[1].second.c_str(), g_basicTestResources[1].second.length()) == 0);
   }

   {
      byte * pFooDat2 = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat2) == S_OK);
      CHECK(memcmp(pFooDat2, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   CHECK_EQUAL(2, m_pDiagnostics->GetCacheSize());
}

////////////////////////////////////////

TEST_FIXTURE(cResourceManagerTests, ResourceManagerLoadCaseSensitivity)
{
   AddTestData(&g_basicTestResources[0], _countof(g_basicTestResources));

   CHECK(AccessResourceManager()->RegisterFormat(kRT_Data, NULL, "dat", RawBytesLoad, NULL, RawBytesUnload) == S_OK);

   {
      byte * pFooDat1 = NULL;
      CHECK(m_pResourceManager->Load("foo", kRT_Data, (void*)NULL, (void**)&pFooDat1) == S_OK);
      CHECK(memcmp(pFooDat1, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }

   {
      byte * pFooDat2 = NULL;
      CHECK(m_pResourceManager->Load("FOO", kRT_Data, (void*)NULL, (void**)&pFooDat2) == S_OK);
      CHECK(memcmp(pFooDat2, g_basicTestResources[0].second.c_str(), g_basicTestResources[0].second.length()) == 0);
   }
}

////////////////////////////////////////

TEST_FIXTURE(cResourceManagerTests, ResourceManagerListResources)
{
   AddTestData(&g_basicTestResources[0], _countof(g_basicTestResources));

   CHECK(AccessResourceManager()->RegisterFormat(kRT_Data, NULL, "dat", RawBytesLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat(kRT_Bitmap, NULL, "bmp", RawBytesLoad, NULL, RawBytesUnload) == S_OK);

   {
      std::vector<cStr> dataResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.dat"), &dataResNames) == S_OK);
      CHECK_EQUAL(dataResNames.size(), 2);
   }

   {
      std::vector<cStr> bitmapResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.bmp"), &bitmapResNames) == S_OK);
      CHECK_EQUAL(bitmapResNames.size(), 1);
   }

   {
      std::vector<cStr> allResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.dat"), &allResNames) == S_OK);
      CHECK(m_pResourceManager->ListResources(_T("*.bmp"), &allResNames) == S_OK);
      CHECK_EQUAL(allResNames.size(), 3);
   }
}

////////////////////////////////////////

const tStrPair g_multNameTestResources[] =
{
   std::make_pair(cStr("foo.xml"), cStr("<?xml version=\"1.0\" ?>...\0")),
   std::make_pair(cStr("foo.ms3d"), cStr("MS3D0000...........\0")),
};

TEST_FIXTURE(cResourceManagerTests, ResourceManagerSameNameLoadWrongType)
{
   AddTestData(&g_multNameTestResources[0], _countof(g_multNameTestResources));

   CHECK(AccessResourceManager()->RegisterFormat("footxt", NULL, "xml", RawBytesLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat("footxt", NULL, "txt", RawBytesLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat("fooms3d", NULL, "ms3d", PseudoMs3dLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat("fooxml", "footxt", "xml", NULL, PseudoXmlPostload, PseudoXmlUnload) == S_OK);

   {
      byte * pFooXml = NULL;
      CHECK(m_pResourceManager->Load("foo.xml", "fooxml", (void*)NULL, (void**)&pFooXml) == S_OK);
      if (pFooXml != NULL)
      {
         const cStr & expected = g_multNameTestResources[0].second;
         CHECK(memcmp(pFooXml, expected.c_str(), expected.length()) == 0);
      }
   }

   // This is a bad call. The resource manager should simply fail it without any ill effects
   {
      byte * pFooMs3d = NULL;
      CHECK(m_pResourceManager->Load("foo.ms3d", "fooxml", (void*)NULL, (void**)&pFooMs3d) != S_OK);
      CHECK(pFooMs3d == NULL);
   }
}

////////////////////////////////////////
// Try loading the same resource as two different types. This should be allowed.
// For example, loading a map file as terrain or as properties.

#if 0 // TODO: this case doesn't work, but should
const tStrPair g_fakeMapResource[] =
{
   std::make_pair(cStr("foo.map"), cStr("SGEAC0AD9D21E34D9ADCF83E4235A2345F\0")),
};

TEST_FIXTURE(cResourceManagerTests, ResourceManagerSameResourceTwoTypes)
{
   AddTestData(&g_fakeMapResource[0], _countof(g_fakeMapResource));

   CHECK(AccessResourceManager()->RegisterFormat("map",       NULL, "map", RawBytesLoad, NULL, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat("mapprops",  NULL, "map", RawBytesLoad, NULL, RawBytesUnload) == S_OK);

   byte * pFooMap = NULL;
   CHECK(m_pResourceManager->Load("foo.map", "map", (void*)NULL, (void**)&pFooMap) == S_OK);

   byte * pFooMapProps = NULL;
   CHECK(m_pResourceManager->Load("foo.map", "mapprops", (void*)NULL, (void**)&pFooMapProps) == S_OK);
}
#endif

////////////////////////////////////////

// This unit test is for a case that the resource manager doesn't actually handle yet
#if 0
const tStrPair g_multExtTestResources[] =
{
   std::make_pair(cStr("foo.ms3d"), cStr("foo_ms3d_foo_ms3d_foo_ms3d_foo_ms3d\0")),
   std::make_pair(cStr("foo.ms3d.xml"), cStr("foo_ms3d_xml_foo_ms3d_xml_foo_ms3d_xml\0")),
};

TEST_FIXTURE(cResourceManagerTests, ResourceManagerMultipleExtensionConfusion)
{
   AddTestData(&g_multExtTestResources[0], _countof(g_multExtTestResources));

   CHECK(AccessResourceManager()->RegisterFormat("fooms3d", NULL, "ms3d", RawBytesLoad, TestDataPostload, RawBytesUnload) == S_OK);
   CHECK(AccessResourceManager()->RegisterFormat("fooxml", NULL, "xml", RawBytesLoad, TestDataPostload, RawBytesUnload) == S_OK);

   {
      byte * pFooXml = NULL;
      CHECK(m_pResourceManager->Load("foo.ms3d", "fooxml", (void*)NULL, (void**)&pFooXml) == S_OK);
      if (pFooXml != NULL)
      {
         const cStr & expected = g_multExtTestResources[1].second;
         CHECK(memcmp(pFooXml, expected.c_str(), expected.length()) == 0);
      }
   }
}
#endif

#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
