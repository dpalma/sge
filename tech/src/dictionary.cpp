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

cDictionary::cDictionary(tPersistence defaultPersist, IUnknown * pUnkOuter)
 : cComAggregableObject<IMPLEMENTS(IDictionary)>(pUnkOuter)
 , m_defaultPersist(defaultPersist)
{
}

///////////////////////////////////////

cDictionary::~cDictionary()
{
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, tChar * pVal, int maxLength, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::const_iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL && maxLength > 0)
      {
         _tcsncpy(pVal, iter->second.ToString(), maxLength);
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

   tMap::const_iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL)
      {
         pVal->assign(iter->second);
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

   tMap::const_iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      int value = iter->second.ToInt();

      if ((value == 0) && !iter->second.IsEmpty())
      {
         return S_FALSE;
      }

      if (pVal != NULL)
      {
         *pVal = value;
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

   tMap::const_iterator iter = m_vars.find(pszKey);
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

tResult cDictionary::Get(const tChar * pszKey, double * pVal, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::const_iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (FAILED(GetPersistence(pszKey, pPersist)))
      {
         DebugMsg1("ERROR: Could not find persistence value for %s\n", pszKey);
         return E_FAIL;
      }

      if (pVal != NULL)
      {
         *pVal = iter->second.ToDouble();
      }

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, cMultiVar * pVal, tPersistence * pPersist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }

   tMap::const_iterator iter = m_vars.find(pszKey);
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

tResult cDictionary::Set(const tChar * pszKey, const tChar * val, tPersistence persist)
{
   if (pszKey == NULL || val == NULL)
   {
      return E_POINTER;
   }
   if (*pszKey == 0)
   {
      return E_INVALIDARG;
   }
   m_vars[cStr(pszKey)] = val;
   m_persistenceMap[pszKey] = (persist != kUseDefault) ? persist : m_defaultPersist;
   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, int val, tPersistence persist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   if (*pszKey == 0)
   {
      return E_INVALIDARG;
   }
   m_vars[pszKey] = val;
   m_persistenceMap[pszKey] = (persist != kUseDefault) ? persist : m_defaultPersist;
   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, float val, tPersistence persist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   if (*pszKey == 0)
   {
      return E_INVALIDARG;
   }
   m_vars[pszKey] = val;
   m_persistenceMap[pszKey] = (persist != kUseDefault) ? persist : m_defaultPersist;
   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, double val, tPersistence persist)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   if (*pszKey == 0)
   {
      return E_INVALIDARG;
   }
   m_vars[pszKey] = val;
   m_persistenceMap[pszKey] = (persist != kUseDefault) ? persist : m_defaultPersist;
   return S_OK;
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

   tMap::const_iterator iter;
   for (iter = m_vars.begin(); iter != m_vars.end(); iter++)
   {
      pKeys->push_back(iter->first);
   }

   return S_OK;
}

///////////////////////////////////////

void cDictionary::Clear()
{
   m_vars.clear();
   m_persistenceMap.clear();
}

///////////////////////////////////////

tResult cDictionary::Clone(IDictionary * * ppDictionary) const
{
   if (ppDictionary == NULL)
   {
      return E_POINTER;
   }

   cDictionary * pDict = new cDictionary(m_defaultPersist);
   if (pDict == NULL)
   {
      return E_OUTOFMEMORY;
   }

#if _MSC_VER <= 1200
   {
      tMap::const_iterator iter = m_vars.begin();
      for (; iter != m_vars.end(); iter++)
      {
         pDict->m_vars.insert(*iter);
      }
   }
   {
      tPersistenceMap::const_iterator iter = m_persistenceMap.begin();
      for (; iter != m_persistenceMap.end(); iter++)
      {
         pDict->m_persistenceMap.insert(*iter);
      }
   }
#else
   pDict->m_vars.insert(m_vars.begin(), m_vars.end());
   pDict->m_persistenceMap.insert(m_persistenceMap.begin(), m_persistenceMap.end());
#endif

   *ppDictionary = static_cast<IDictionary*>(pDict);
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
      tPersistenceMap::const_iterator piter = m_persistenceMap.find(pszKey);
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

///////////////////////////////////////

IUnknown * DictionaryCreate(tPersistence defaultPersist, IUnknown * pUnkOuter)
{
   cDictionary * pDict = new cDictionary(defaultPersist, pUnkOuter);
   if (pDict != NULL)
   {
      return pDict->AccessInnerUnknown();
   }
   return NULL;
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
   cStr key, value;

   for (i = 0; i < kNumTestVars; i++)
   {
      Sprintf(&key, szPermKeyFormat, i);
      Sprintf(&value, szPermValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Set(key.c_str(), value.c_str(), kPermanent) == S_OK);

      Sprintf(&key, szTempKeyFormat, i);
      Sprintf(&value, szTempValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Set(key.c_str(), value.c_str(), kTransitory) == S_OK);
   }

   for (i = 0; i < kNumTestVars; i++)
   {
      cStr value2;
      tPersistence persist;

      Sprintf(&key, szPermKeyFormat, i);
      Sprintf(&value, szPermValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Get(key.c_str(), &value2, &persist) == S_OK);
      CPPUNIT_ASSERT(strcmp(value2.c_str(), value.c_str()) == 0);
      CPPUNIT_ASSERT(persist == kPermanent);

      Sprintf(&key, szTempKeyFormat, i);
      Sprintf(&value, szTempValFormat, i);
      CPPUNIT_ASSERT(pSaveDict->Get(key.c_str(), &value2, &persist) == S_OK);
      CPPUNIT_ASSERT(strcmp(value2.c_str(), value.c_str()) == 0);
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
         cStr value2;
         tPersistence persist;

         Sprintf(&key, szPermKeyFormat, i);
         Sprintf(&value, szPermValFormat, i);
         CPPUNIT_ASSERT(pLoadDict->Get(key.c_str(), &value2, &persist) == S_OK);
         CPPUNIT_ASSERT(strcmp(value2.c_str(), value.c_str()) == 0);
         CPPUNIT_ASSERT(persist == kPermanent);

         Sprintf(&key, szTempKeyFormat, i);
         Sprintf(&value, szTempValFormat, i);
         CPPUNIT_ASSERT(pLoadDict->Get(key.c_str(), &value2, &persist) == S_FALSE);
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
