///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techstring.h"
#include "combase.h"

#include <cstdlib>
#include <cstring>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStr
//

const cStr cStr::gm_whitespace(" \t\r\n");

///////////////////////////////////////

int cStr::ParseTuple(std::vector<cStr> * pStrings) const
{
   if (pStrings == NULL)
   {
      return E_POINTER;
   }

   pStrings->clear();

   const char * psz = Get();

   const char * pszStop = psz + strlen(psz);

   // the open and close bracket strings must "line up"
   // for example, '<' and '>' both appear at index zero
   static const char szOpenBrackets[]  = "<([{";
   static const char szCloseBrackets[] = ">)]}";

   const char * pszOpenBracket = strchr(szOpenBrackets, *psz);
   if (pszOpenBracket != NULL)
   {
      if (*(pszStop - 1) == szCloseBrackets[pszOpenBracket - szOpenBrackets])
      {
         psz++;
         pszStop--;
      }
      else
      {
         // Unmatched brackets
         return E_INVALIDARG;
      }
   }

   int nParsed = 0;

   while (psz < pszStop)
   {
      while (isspace(*psz))
      {
         psz++;
      }

      const char * psz2 = strpbrk(psz, ",;");
      if (psz2 == NULL)
      {
         psz2 = pszStop;
      }

      int len = psz2 - psz;

      if (len > 0)
      {
         pStrings->push_back(cStr(psz, len));
         nParsed++;
      }

      psz = psz2 + 1;
   }

   Assert(nParsed == pStrings->size());

   return nParsed;
}

///////////////////////////////////////

int cStr::ParseTuple(double * pDoubles, int nMaxDoubles) const
{
   if (pDoubles == NULL)
   {
      return E_POINTER;
   }

   if (nMaxDoubles == 0)
   {
      return E_INVALIDARG;
   }

   const char * psz = Get();

   std::vector<cStr> strings;
   int result = ParseTuple(&strings);
   if (result > 0)
   {
      int count = 0;
      std::vector<cStr>::iterator iter = strings.begin();
      std::vector<cStr>::iterator end = strings.end();
      for (; (iter != end) && (count < nMaxDoubles); iter++, count++)
      {
         *pDoubles++ = iter->ToFloat();
      }
      return count;
   }

   return result;
}

///////////////////////////////////////

int cStr::ParseTuple(float * pFloats, int nMaxFloats) const
{
   if (pFloats == NULL)
   {
      return E_POINTER;
   }

   if (nMaxFloats == 0)
   {
      return E_INVALIDARG;
   }

   const char * psz = Get();

   std::vector<cStr> strings;
   int result = ParseTuple(&strings);
   if (result > 0)
   {
      int count = 0;
      std::vector<cStr>::iterator iter = strings.begin();
      std::vector<cStr>::iterator end = strings.end();
      for (; (iter != end) && (count < nMaxFloats); iter++, count++)
      {
         *pFloats++ = iter->ToFloat();
      }
      return count;
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cStrTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cStrTests);
      CPPUNIT_TEST(TestParseBadArgs);
      CPPUNIT_TEST(TestParseSuccessCases);
   CPPUNIT_TEST_SUITE_END();

   void TestParseBadArgs();
   void TestParseSuccessCases();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cStrTests);

////////////////////////////////////////

void cStrTests::TestParseBadArgs()
{
   CPPUNIT_ASSERT(cStr(NULL).ParseTuple((double *)0xDEADBEEF, 1) == E_INVALIDARG);
   CPPUNIT_ASSERT(cStr("blah").ParseTuple((double *)NULL, 1) == E_INVALIDARG);
   double n[4];
   CPPUNIT_ASSERT(cStr("[1,2]").ParseTuple(n, 0) == E_INVALIDARG);
   CPPUNIT_ASSERT(cStr("[1,2,3").ParseTuple(n, _countof(n)) == E_INVALIDARG);
}

////////////////////////////////////////

void cStrTests::TestParseSuccessCases()
{
   double n[4];

   CPPUNIT_ASSERT(cStr("[]").ParseTuple(n, _countof(n)) == 0);
   CPPUNIT_ASSERT(cStr("{  }").ParseTuple(n, _countof(n)) == 0);

   CPPUNIT_ASSERT(cStr("[1,2,3,4]").ParseTuple(n, _countof(n)) == 4);
   CPPUNIT_ASSERT(n[0] == 1);
   CPPUNIT_ASSERT(n[1] == 2);
   CPPUNIT_ASSERT(n[2] == 3);
   CPPUNIT_ASSERT(n[3] == 4);

   CPPUNIT_ASSERT(cStr("< 10 ; 32.2 ; 48.8 >").ParseTuple(n,  _countof(n)) == 3);
   CPPUNIT_ASSERT(n[0] == 10);
   CPPUNIT_ASSERT(n[1] == 32.2);
   CPPUNIT_ASSERT(n[2] == 48.8);

   CPPUNIT_ASSERT(cStr("(1,2,3,4)").ParseTuple(n, 2) == 2);
   CPPUNIT_ASSERT(n[0] == 1);
   CPPUNIT_ASSERT(n[1] == 2);

   CPPUNIT_ASSERT(cStr("1000,2000,3000,4000").ParseTuple(n, 3) == 3);
   CPPUNIT_ASSERT(n[0] == 1000);
   CPPUNIT_ASSERT(n[1] == 2000);
   CPPUNIT_ASSERT(n[2] == 3000);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
