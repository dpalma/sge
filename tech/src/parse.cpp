///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "parse.h"

#include <cstdlib>
#include <cstring>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

int ParseTuple(const char * psz, double * pNumbers, int nMaxNumbers)
{
   if (psz == NULL || *psz == 0 || pNumbers == NULL || nMaxNumbers == 0)
      return kPTE_InvalidArg;

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
         return kPTE_UnmatchedBrackets;
      }
   }

   int nNumbers = 0;

   while ((psz < pszStop) && nMaxNumbers-- > 0)
   {
      while (isspace(*psz))
         psz++;

      const char * psz2 = strpbrk(psz, ",;");
      if (psz2 == NULL)
         psz2 = pszStop;

      int len = psz2 - psz;

      if (len > 0)
      {
         char * pszTemp = static_cast<char *>(alloca(len + 1));
         strncpy(pszTemp, psz, len);
         pszTemp[len] = 0;

         *pNumbers++ = atof(pszTemp);

         nNumbers++;
      }

      psz = psz2 + 1;
   }

   return nNumbers;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cParseTupleTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cParseTupleTests);
      CPPUNIT_TEST(TestBadArgs);
      CPPUNIT_TEST(TestSuccessCases);
   CPPUNIT_TEST_SUITE_END();

   void TestBadArgs()
   {
      CPPUNIT_ASSERT(ParseTuple(NULL, (double *)0xDEADBEEF, 1) == kPTE_InvalidArg);
      CPPUNIT_ASSERT(ParseTuple("blah", NULL, 1) == kPTE_InvalidArg);
      double n[4];
      CPPUNIT_ASSERT(ParseTuple("[1,2]", n, 0) == kPTE_InvalidArg);
      CPPUNIT_ASSERT(ParseTuple("[1,2,3", n, _countof(n)) == kPTE_UnmatchedBrackets);
   }

   void TestSuccessCases()
   {
      double n[4];

      CPPUNIT_ASSERT(ParseTuple("[]", n, _countof(n)) == 0);
      CPPUNIT_ASSERT(ParseTuple("{  }", n, _countof(n)) == 0);

      CPPUNIT_ASSERT(ParseTuple("[1,2,3,4]", n, _countof(n)) == 4);
      CPPUNIT_ASSERT(n[0] == 1);
      CPPUNIT_ASSERT(n[1] == 2);
      CPPUNIT_ASSERT(n[2] == 3);
      CPPUNIT_ASSERT(n[3] == 4);

      CPPUNIT_ASSERT(ParseTuple("< 10 ; 32.2 ; 48.8 >", n,  _countof(n)) == 3);
      CPPUNIT_ASSERT(n[0] == 10);
      CPPUNIT_ASSERT(n[1] == 32.2);
      CPPUNIT_ASSERT(n[2] == 48.8);

      CPPUNIT_ASSERT(ParseTuple("(1,2,3,4)", n, 2) == 2);
      CPPUNIT_ASSERT(n[0] == 1);
      CPPUNIT_ASSERT(n[1] == 2);

      CPPUNIT_ASSERT(ParseTuple("1000,2000,3000,4000", n, 3) == 3);
      CPPUNIT_ASSERT(n[0] == 1000);
      CPPUNIT_ASSERT(n[1] == 2000);
      CPPUNIT_ASSERT(n[2] == 3000);
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cParseTupleTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
