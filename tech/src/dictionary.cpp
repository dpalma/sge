///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "dictionary.h"

#include <cstdio>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDictionary
//

///////////////////////////////////////

bool cDictionary::cStringLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (stricmp(lhs.c_str(), rhs.c_str()) < 0) ? true : false;
}

///////////////////////////////////////

cDictionary::cDictionary()
{
}

///////////////////////////////////////

cDictionary::~cDictionary()
{
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, char * pVal, int maxLength)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
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

tResult cDictionary::Get(const tChar * pszKey, cStr * pVal)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
      {
         *pVal = iter->second.c_str();
      }
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, int * pVal)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
      {
         *pVal = atoi(iter->second.c_str());
      }
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Get(const tChar * pszKey, float * pVal)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tMap::iterator iter = m_vars.find(pszKey);
   if (iter != m_vars.end())
   {
      if (pVal != NULL)
      {
         *pVal = (float)atof(iter->second.c_str());
      }
      return S_OK;
   }
   return S_FALSE;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, const char * val)
{
   if (pszKey == NULL || val == NULL)
   {
      return E_POINTER;
   }
   m_vars[pszKey] = val;
   return S_OK;
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, int val)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tChar szBuffer[64];
   snprintf(szBuffer, _countof(szBuffer), "%d", val);
   return Set(pszKey, szBuffer);
}

///////////////////////////////////////

tResult cDictionary::Set(const tChar * pszKey, float val)
{
   if (pszKey == NULL)
   {
      return E_POINTER;
   }
   tChar szBuffer[64];
   snprintf(szBuffer, _countof(szBuffer), "%f", val);
   return Set(pszKey, szBuffer);
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

IDictionary * DictionaryCreate()
{
   return static_cast<IDictionary *>(new cDictionary);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cDictionaryTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cDictionaryTests);
      CPPUNIT_TEST(TestBadArgs);
      CPPUNIT_TEST(TestSet);
   CPPUNIT_TEST_SUITE_END();

   void TestBadArgs();
   void TestSet();
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

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
