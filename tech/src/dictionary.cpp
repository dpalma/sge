///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "dictionary.h"
#include "filespec.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstdio>

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

tResult cDictionary::Set(const tChar * pszKey, const char * val, tPersistence persist)
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

tResult cDictionary::Set(const tChar * pszKey, const wchar_t * val, tPersistence persist)
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

#if _MSC_VER > 1200
   pDict->m_vars.insert(m_vars.begin(), m_vars.end());
   pDict->m_persistenceMap.insert(m_persistenceMap.begin(), m_persistenceMap.end());
#else
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

///////////////////////////////////////////////////////////////////////////////

tResult DictionaryCreate(IDictionary * * ppDictionary)
{
   return DictionaryCreate(kPermanent, ppDictionary);
}

///////////////////////////////////////////////////////////////////////////////

tResult DictionaryCreate(tPersistence persist, IDictionary * * ppDictionary)
{
   if (ppDictionary == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IDictionary> pDictionary(static_cast<IDictionary *>(new cDictionary(persist)));
   if (!pDictionary)
   {
      return E_OUTOFMEMORY;
   }

   return pDictionary.GetPointer(ppDictionary);
}

///////////////////////////////////////////////////////////////////////////////

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

#ifdef HAVE_UNITTESTPP

///////////////////////////////////////

TEST(DictionaryBadArgs)
{
   cAutoIPtr<IDictionary> pDict;
   CHECK(DictionaryCreate(&pDict) == S_OK);
   CHECK(FAILED(pDict->Get(NULL, (tChar *)NULL, 0)));
   CHECK(FAILED(pDict->Get(NULL, (cStr *)NULL)));
   CHECK(FAILED(pDict->Get(NULL, (int *)NULL)));
   CHECK(FAILED(pDict->Get(NULL, (float *)NULL)));
   CHECK(FAILED(pDict->Set(NULL, NULL)));
   CHECK(FAILED(pDict->Set(NULL, 999)));
   CHECK(FAILED(pDict->Set(NULL, 999.999f)));
   CHECK(FAILED(pDict->Delete(NULL)));
   CHECK(FAILED(pDict->IsSet(NULL)));
   CHECK(FAILED(pDict->GetKeys(NULL)));
}

///////////////////////////////////////

TEST(DictionarySet)
{
   static const char szKey[] = "test";
   static const char szValue1[] = "value one";
   static const char szValue2[] = "value two";

   cStr value;

   cAutoIPtr<IDictionary> pDict;
   CHECK(DictionaryCreate(&pDict) == S_OK);

   // set value one
   CHECK(pDict->Set(szKey, szValue1) == S_OK);

   // verify
   CHECK(pDict->Get(szKey, &value) == S_OK);
   CHECK(strcmp(value.c_str(), szValue1) == 0);

   // overwrite with value two
   CHECK(pDict->Set(szKey, szValue2) == S_OK);

   // verify
   CHECK(pDict->Get(szKey, &value) == S_OK);
   CHECK(strcmp(value.c_str(), szValue2) == 0);
}

///////////////////////////////////////

TEST(DictionaryPersistence)
{
   static const int kNumTestVars = 4;
   static const char szPermKeyFormat[] = "perm%d";
   static const char szPermValFormat[] = "pval%d";
   static const char szTempKeyFormat[] = "temp%d";
   static const char szTempValFormat[] = "tval%d";

   cAutoIPtr<IDictionary> pSaveDict;
   CHECK(DictionaryCreate(&pSaveDict) == S_OK);

   int i;
   cStr key, value;

   for (i = 0; i < kNumTestVars; i++)
   {
      Sprintf(&key, szPermKeyFormat, i);
      Sprintf(&value, szPermValFormat, i);
      CHECK(pSaveDict->Set(key.c_str(), value.c_str(), kPermanent) == S_OK);

      Sprintf(&key, szTempKeyFormat, i);
      Sprintf(&value, szTempValFormat, i);
      CHECK(pSaveDict->Set(key.c_str(), value.c_str(), kTransitory) == S_OK);
   }

   for (i = 0; i < kNumTestVars; i++)
   {
      cStr value2;
      tPersistence persist;

      Sprintf(&key, szPermKeyFormat, i);
      Sprintf(&value, szPermValFormat, i);
      CHECK(pSaveDict->Get(key.c_str(), &value2, &persist) == S_OK);
      CHECK(strcmp(value2.c_str(), value.c_str()) == 0);
      CHECK(persist == kPermanent);

      Sprintf(&key, szTempKeyFormat, i);
      Sprintf(&value, szTempValFormat, i);
      CHECK(pSaveDict->Get(key.c_str(), &value2, &persist) == S_OK);
      CHECK(strcmp(value2.c_str(), value.c_str()) == 0);
      CHECK(persist == kTransitory);
   }

   tChar szStore[TMP_MAX];
#if _MSC_VER >= 1300
   CHECK(_ttmpnam_s(szStore, _countof(szStore)) != NULL);
#else
   CHECK(_ttmpnam(szStore) != NULL);
#endif

   try
   {
      cAutoIPtr<IDictionaryStore> pSaveStore(DictionaryStoreCreate(cFileSpec(szStore)));
      CHECK(pSaveStore->Save(pSaveDict) == S_OK);
      SafeRelease(pSaveStore);
      SafeRelease(pSaveDict);

      cAutoIPtr<IDictionary> pLoadDict;
      CHECK(DictionaryCreate(&pLoadDict) == S_OK);
      cAutoIPtr<IDictionaryStore> pLoadStore(DictionaryStoreCreate(cFileSpec(szStore)));
      CHECK(pLoadStore->Load(pLoadDict) == S_OK);

      for (i = 0; i < kNumTestVars; i++)
      {
         cStr value2;
         tPersistence persist;

         Sprintf(&key, szPermKeyFormat, i);
         Sprintf(&value, szPermValFormat, i);
         CHECK(pLoadDict->Get(key.c_str(), &value2, &persist) == S_OK);
         CHECK(strcmp(value2.c_str(), value.c_str()) == 0);
         CHECK(persist == kPermanent);

         Sprintf(&key, szTempKeyFormat, i);
         Sprintf(&value, szTempValFormat, i);
         CHECK(pLoadDict->Get(key.c_str(), &value2, &persist) == S_FALSE);
      }

      CHECK(_unlink(szStore) == 0);
   }
   catch (...)
   {
      // attempt to ensure the temp file is always deleted
      _unlink(szStore);
      throw;
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
