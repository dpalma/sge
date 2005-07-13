///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "configapi.h"
#include "techstring.h"

#include <cstdio>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

extern bool ParseDictionaryLine(const tChar * psz, cStr * pKey, cStr * pValue, cStr * pComment);

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IConfig> g_pAutoDeleteConfig(DictionaryCreate());
IConfig * g_pConfig = static_cast<IConfig *>(g_pAutoDeleteConfig);

static const tChar kTrueStr[] = _T("true");

//////////////////////////////////////////////////////////////////////////////

tResult ParseCommandLine(int argc, tChar *argv[], IConfig * pConfig)
{
   if (argv == NULL || pConfig == NULL)
   {
      return E_POINTER;
   }

   cStr curKey, curVal;
   bool bLastWasKey = false;

   for (int i = 0; i < argc; i++)
   {
      if (argv[i][0] == '+')
      {
         // set currently accumulating key/value
         if (curKey.GetLength() > 0)
         {
            pConfig->Set(curKey.c_str(), curVal.c_str(), kTransitory);
            curKey.Empty();
            curVal.Empty();
         }

         cStr key, value;
         if (ParseDictionaryLine(argv[i] + 1, &key, &value, NULL) && !key.IsEmpty())
         {
            if (value.IsEmpty())
            {
               curKey = key;
               curVal = _T("1");
               bLastWasKey = true;
            }
            else
            {
               pConfig->Set(key.c_str(), value.c_str(), kTransitory);
               curKey.Empty();
               curVal.Empty();
               bLastWasKey = false;
            }
         }
      }
      else if (argv[i][0] == '-')
      {
         pConfig->Delete(argv[i] + 1);
         bLastWasKey = false;
      }
      else
      {
         if (!bLastWasKey)
         {
            curVal.Append(_T(" "));
            curVal.Append(argv[i]);
         }
         else
         {
            curVal = argv[i];
         }
         bLastWasKey = false;
      }
   }

   // set final key
   if (bLastWasKey && curKey.GetLength() > 0)
   {
      pConfig->Set(curKey.c_str(), curVal.c_str(), kTransitory);
   }

   return S_OK;
}

static bool StringIsTrue(const tChar * psz)
{
   Assert(psz != NULL);
   if (_tcsicmp(psz, kTrueStr) == 0)
   {
      return true;
   }
   else if (_tcsicmp(psz, kTrueStr) == 0)
   {
      return true;
   }
   return _ttoi(psz) ? true : false;
}

bool ConfigIsTrue(const tChar * pszName)
{
   if (g_pConfig != NULL)
   {
      cStr value;
      if (g_pConfig->Get(pszName, &value) == S_OK)
      {
         return StringIsTrue(value.c_str());
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cConfigTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cConfigTests);
      CPPUNIT_TEST(TestParseCmdLine);
      CPPUNIT_TEST(TestStringIsTrue);
   CPPUNIT_TEST_SUITE_END();

   void TestParseCmdLine();
   void TestStringIsTrue();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cConfigTests);

///////////////////////////////////////

void cConfigTests::TestParseCmdLine()
{
   cAutoIPtr<IDictionary> pDict(DictionaryCreate());
   pDict->Set("test1", "val1", kTransitory);
   pDict->Set("test2", "val2", kTransitory);
   static char *argv[] =
   {
      "+data=../data",
      "-test1",
      "+test3 = val3",
   };
   CPPUNIT_ASSERT(ParseCommandLine(_countof(argv), argv, pDict) == S_OK);
   CPPUNIT_ASSERT(pDict->IsSet("test1") == S_FALSE);
   cStr temp;
   tPersistence persist;
   CPPUNIT_ASSERT(pDict->Get("test2", &temp, &persist) == S_OK);
   CPPUNIT_ASSERT(strcmp(temp.c_str(), "val2") == 0);
   CPPUNIT_ASSERT(persist == kTransitory);
   persist = kUseDefault;
   temp.erase();
   CPPUNIT_ASSERT(pDict->Get("test3", &temp, &persist) == S_OK);
   CPPUNIT_ASSERT(strcmp(temp.c_str(), "val3") == 0);
   CPPUNIT_ASSERT(persist == kTransitory);
}

///////////////////////////////////////

void cConfigTests::TestStringIsTrue()
{
   CPPUNIT_ASSERT(StringIsTrue("true"));
   CPPUNIT_ASSERT(!StringIsTrue("false"));
   CPPUNIT_ASSERT(StringIsTrue("TRUE"));
   CPPUNIT_ASSERT(!StringIsTrue("FALSE"));
   CPPUNIT_ASSERT(StringIsTrue("1"));
   CPPUNIT_ASSERT(!StringIsTrue("0"));
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
