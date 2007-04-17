////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#ifdef HAVE_UNITTESTPP

#include "resourcemanager.h"
#include "resourcestore.h"

#include "tech/readwriteapi.h"

#include "UnitTest++.h"

#include "tech/dbgalloc.h" // must be last header

using namespace std;

///////////////////////////////////////////////////////////////////////////////

typedef pair<cStr, cStr> tStrPair;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTestResourceStore
//

class cTestResourceStore : public cComObject<IMPLEMENTS(IResourceStore)>
{
public:
   cTestResourceStore(const tStrPair * pTestData, size_t nTestData);
   virtual ~cTestResourceStore();

   virtual tResult CollectResourceNames(const tChar * pszMatch, vector<cStr> * pNames);
   virtual tResult OpenEntry(const tChar * pszName, IReader * * ppReader);

private:
   // Pairs of <file name, pseudo data>
   vector<tStrPair> m_testData;
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

tResult cTestResourceStore::CollectResourceNames(const tChar * pszMatch, vector<cStr> * pNames)
{
   vector<pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
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
   vector<pair<cStr, cStr> >::const_iterator iter = m_testData.begin();
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

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cResourceManagerTests
//

////////////////////////////////////////

class cResourceManagerTests
{
   cResourceManagerTests(const cResourceManagerTests &);
   const cResourceManagerTests & operator =(const cResourceManagerTests &);

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
      m_pResourceManager->m_stores.push_back(static_cast<IResourceStore*>(pStore));
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
   make_pair(cStr("foo.dat"), cStr("foo_dat_foo_dat_foo_dat_foo_dat\0")),
   make_pair(cStr("foo.bmp"), cStr("foo_bmp_foo_bmp_foo_bmp_foo_bmp\0")),
   make_pair(cStr("bar.dat"), cStr("bar_dat_bar_dat_bar_dat_bar_dat\0")),
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
      vector<cStr> dataResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.dat"), &dataResNames) == S_OK);
      CHECK_EQUAL(dataResNames.size(), 2);
   }

   {
      vector<cStr> bitmapResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.bmp"), &bitmapResNames) == S_OK);
      CHECK_EQUAL(bitmapResNames.size(), 1);
   }

   {
      vector<cStr> allResNames;
      CHECK(m_pResourceManager->ListResources(_T("*.dat"), &allResNames) == S_OK);
      CHECK(m_pResourceManager->ListResources(_T("*.bmp"), &allResNames) == S_OK);
      CHECK_EQUAL(allResNames.size(), 3);
   }
}

////////////////////////////////////////

const tStrPair g_multNameTestResources[] =
{
   make_pair(cStr("foo.xml"), cStr("<?xml version=\"1.0\" ?>...\0")),
   make_pair(cStr("foo.ms3d"), cStr("MS3D0000...........\0")),
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
   make_pair(cStr("foo.map"), cStr("SGEAC0AD9D21E34D9ADCF83E4235A2345F\0")),
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
   make_pair(cStr("foo.ms3d"), cStr("foo_ms3d_foo_ms3d_foo_ms3d_foo_ms3d\0")),
   make_pair(cStr("foo.ms3d.xml"), cStr("foo_ms3d_xml_foo_ms3d_xml_foo_ms3d_xml\0")),
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
