///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "configapi.h"
#include "str.h"

#include <cstdio>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

extern bool ParseDictionaryLine(const tChar * psz, cStr * pKey, cStr * pValue, cStr * pComment);

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IConfig> g_pAutoDeleteConfig(DictionaryCreate());
IConfig * g_pConfig = static_cast<IConfig *>(g_pAutoDeleteConfig);

//////////////////////////////////////////////////////////////////////////////

tResult ParseCommandLine(int argc, char *argv[], IConfig * pConfig)
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
            pConfig->Set(curKey, curVal);
            curKey.erase();
            curVal.erase();
         }

         cStr key, value;
         if (ParseDictionaryLine(argv[i] + 1, &key, &value, NULL) && !key.empty())
         {
            curKey = key;
            curVal = value.empty() ? value : "1";
            bLastWasKey = true;
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
            curVal += " ";
            curVal += argv[i];
         }
         else
         {
            curVal = argv[i];
         }
         bLastWasKey = false;
      }
   }

   // set final key
   if (curKey.GetLength() > 0)
   {
      pConfig->Set(curKey, curVal);
   }

   return S_OK;
}

static bool StringIsTrue(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, "true") == 0)
   {
      return true;
   }
   else if (stricmp(psz, "true") == 0)
   {
      return true;
   }
   return atoi(psz) ? true : false;
}

bool ConfigIsTrue(const char * pszName)
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
   // TODO
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
