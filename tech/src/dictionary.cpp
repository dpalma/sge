///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "dictionary.h"

#include <cstdio>

#ifdef HAVE_CPPUNIT
#include "filespec.h"
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionary
//

///////////////////////////////////////

cDictionary::cDictionary(tPersistence defaultPersist)
 : m_defaultPersist(defaultPersist)
{
}

///////////////////////////////////////

cDictionary::~cDictionary()
{
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, char * pVal, int maxLength, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL && maxLength > 0)
      {
         strncpy(pVal, iter->second.c_str(), maxLength);
         pVal[maxLength - 1] = 0;
      }

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, cStr * pVal, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL)
      {
         *pVal = iter->second;
      }

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, int * pVal, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL)
      {
         *pVal = iter->second.ToInt();
      }

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, float * pVal, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL)
      {
         *pVal = iter->second.ToFloat();
      }

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, const char * val, tPersistence persist)
{
   if (pszKey == NULL || val == NULL)
   {
      return E_POINTER;
   }
   if (persist == kUseDefault)
   {
      persist = m_defaultPersist;
   }
   else if (persist != kPermanent && persist != kTransitory)
   {
      DebugMsg1("ERROR: Invalid persistence argument %d\n", persist);
      return E_INVALIDARG;
   }
   m_vars[pszKey] = val;
   m_persistenceMap[pszKey] = persist;
   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, int val, tPersistence persist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tChar szBuffer[64];
   snprintf(szBuffer, _countof(szBuffer), "%d", val);
   return Set(pszKey, szBuffer, persist);
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, float val, tPersistence persist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tChar szBuffer[64];
   snprintf(szBuffer, _countof(szBuffer), "%f", val);
   return Set(pszKey, szBuffer, persist);
}

///////////////////////////////////////

tResult cDictionary::Delete(const tChar * pszKey)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   return (m_vars.erase(pszKey) > 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::IsSet(const tChar * pszKey)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   return (m_vars.find(pszKey) != m_vars.end()) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::GetKeys(std::list<cStr> * pKeys)
{
   if (pKeys == NULL)
   {
      return E_POINTER;
   }

   if (m_vars.size() == 0)
   {
      return S_FALSE;
   }

   tMap::iterator iter;
   for (iter = m_vars.begin(); iter != m_vars.end(); iter++)
   {
      pKeys->push_back(iter->first);
   }

   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::GetPersistence(const tChar * pszKey, tPersistence * pPersist)
{
   if (pPersist == NULL)
   {
      return S_FALSE;
   }
   else
   {
      tPersistenceMap::iterator piter = m_persistenceMap.find(pszKey);
      if (piter != m_persistenceMap.end())
      {
         *pPersist = piter->second;
         return S_OK;
      }
      else
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }
   }
}

///////////////////////////////////////

IDictionary * DictionaryCreate(tPersistence defaultPersist)
{
   return static_cast<IDictionary *>(new cDictionary(defaultPersist));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cDictionaryTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cDictionaryTests);
      CPPUNIT_TEST(TestBadArgs);
      CPPUNIT_TEST(TestSet);
      CPPUNIT_TEST(TestPersistence);
   CPPUNIT_TEST_SUITE_END();

   void TestBadArgs();
   void TestSet();
   void TestPersistence();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cDictionaryTests);

///////////////////////////////////////

void cDictionaryTests::TestBadArgs()
{
   cAutoIPtr<IDictionary> pDict(DictionaryCreate());
   CPPUNIT_ASSERT(FAILED(pDict->Get(NULL, (tChar *)NULL, 0)));
   CPPUNIT_ASSERT(FAILED(pDict->Get(NULL, (cStr *)NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->Get(NULL, (int *)NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->Get(NULL, (float *)NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->Set(NULL, NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->Set(NULL, 999)));
   CPPUNIT_ASSERT(FAILED(pDict->Set(NULL, 999.999f)));
   CPPUNIT_ASSERT(FAILED(pDict->Delete(NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->IsSet(NULL)));
   CPPUNIT_ASSERT(FAILED(pDict->GetKeys(NULL)));
}

///////////////////////////////////////

void cDictionaryTests::TestSet()
{
   static const char szKey[] = "test";
   static const char szValue1[] = "value one";
   static const char szValue2[] = "value two";

   cStr value;

   cAutoIPtr<IDictionary> pDict(DictionaryCreate());

   // set value one
   CPPUNIT_ASSERT(pDict->Set(szKey, szValue1) == S_OK);

   // verify
   CPPUNIT_ASSERT(pDict->Get(szKey, &value) == S_OK);
   CPPUNIT_ASSERT(strcmp(value.c_str(), szValue1) == 0);

   // overwrite with value two
   CPPUNIT_ASSERT(pDict->Set(szKey, szValue2) == S_OK);

   // verify
   CPPUNIT_ASSERT(pDict->Get(szKey, &value) == S_OK);
   CPPUNIT_ASSERT(strcmp(value.c_str(), szValue2) == 0);
}

///////////////////////////////////////

void cDictionaryTests::TestPersistence()
{
   static const int kNumTestVars = 4;
   static const char szPermKeyFormat[] = "perm%d";
   static const char szPermValFormat[] = "pval%d";
   static const char szTempKeyFormat[] = "temp%d";
   static const char szTempValFormat[] = "tval%d";

   cAutoIPtr<IDictionary> pSaveDict(DictionaryCreate());

   int i;
   char szKey[100], szValue[100];

   for (i = 0; i < kNumTestVars; i++)
   {
      snprintf(szKey, _countof(szKey), szPermKeyFormat, i);
      snprintf(szValue, _countof(szValue), szPermValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Set(szKey, szValue, kPermanent) == S_OK);

      snprintf(szKey, _countof(szKey), szTempKeyFormat, i);
      snprintf(szValue, _countof(szValue), szTempValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Set(szKey, szValue, kTransitory) == S_OK);
   }

   for (i = 0; i < kNumTestVars; i++)
   {
      cStr value;
      tPersistence persist;

      snprintf(szKey, _countof(szKey), szPermKeyFormat, i);
      snprintf(szValue, _countof(szValue), szPermValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Get(szKey, &value, &persist) == S_OK);
      CPPUNIT_ASSERT(strcmp(value.c_str(), szValue) == 0);
      CPPUNIT_ASSERT(persist == kPermanent);

      snprintf(szKey, _countof(szKey), szTempKeyFormat, i);
      snprintf(szValue, _countof(szValue), szTempValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Get(szKey, &value, &persist) == S_OK);
      CPPUNIT_ASSERT(strcmp(value.c_str(), szValue) == 0);
      CPPUNIT_ASSERT(persist == kTransitory);
   }

   char szStore[TMP_MAX];
   CPPUNIT_ASSERT(tmpnam(szStore) != NULL);

   try
   {
      cAutoIPtr<IDictionaryStore> pSaveStore(DictionaryStoreCreate(cFileSpec(szStore)));
      CPPUNIT_ASSERT(pSaveStore->Save(pSaveDict) == S_OK);
      SafeRelease(pSaveStore);
      SafeRelease(pSaveDict);

      cAutoIPtr<IDictionary> pLoadDict(DictionaryCreate());
      cAutoIPtr<IDictionaryStore> pLoadStore(DictionaryStoreCreate(cFileSpec(szStore)));
      CPPUNIT_ASSERT(pLoadStore->Load(pLoadDict) == S_OK);

      for (i = 0; i < kNumTestVars; i++)
      {
         cStr value;
         tPersistence persist;

         snprintf(szKey, _countof(szKey), szPermKeyFormat, i);
         snprintf(szValue, _countof(szValue), szPermValFormat, i);
         CPPUNIT_ASSERT(pLoadDict->Get(szKey, &value, &persist) == S_OK);
         CPPUNIT_ASSERT(strcmp(value.c_str(), szValue) == 0);
         CPPUNIT_ASSERT(persist == kPermanent);

         snprintf(szKey, _countof(szKey), szTempKeyFormat, i);
         snprintf(szValue, _countof(szValue), szTempValFormat, i);
         CPPUNIT_ASSERT(pLoadDict->Get(szKey, &value, &persist) == S_FALSE);
      }

      CPPUNIT_ASSERT(unlink(szStore) == 0);
   }
   catch (...)
   {
      // attempt to ensure the temp file is always deleted
      unlink(szStore);
      throw;
   }
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
