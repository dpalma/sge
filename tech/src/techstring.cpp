///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techstring.h"
#include "combase.h"

#include <cstdlib>
#include <cstring>
#include <cstdarg>
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

///////////////////////////////////////

#ifdef _MSC_VER
inline int FormatLength(const tChar * pszFormat, va_list args)
{
   return _vscprintf(pszFormat, args);
}
#else
#error ("Need platform-specific implementation of _vscprintf")
#endif

///////////////////////////////////////

int CDECL cStr::Format(const tChar * pszFormat, ...)
{
   va_list args;
   va_start(args, pszFormat);
   int length = FormatLength(pszFormat, args) + 1; // plus one for null terminator
   tChar * pszTemp = reinterpret_cast<tChar*>(alloca(length * sizeof(tChar)));
   _vsnprintf(pszTemp, length, pszFormat, args);
   va_end(args);
   *this = cStr(pszTemp);
   return length;
}

///////////////////////////////////////////////////////////////////////////////

static cStr FilteredCopy(const cStr & str, const cStr & excluded)
{
   cStr result;
   for (uint i = 0; i < str.GetLength(); ++i)
   {
      if (excluded.find(str[i]) == cStr::npos)
      {
         result.append(1, str[i]);
      }
   }
   return result;
}

///////////////////////////////////////////////////////////////////////////////

static const cStr g_fileSeps("\\/");

int filepathcmp(const cStr & f1, const cStr & f2)
{
   cStr cf1(FilteredCopy(f1, g_fileSeps));
   cStr cf2(FilteredCopy(f2, g_fileSeps));
   return strcmp(cf1.c_str(), cf2.c_str());
}

///////////////////////////////////////////////////////////////////////////////

int filepathicmp(const cStr & f1, const cStr & f2)
{
   cStr cf1(FilteredCopy(f1, g_fileSeps));
   cStr cf2(FilteredCopy(f2, g_fileSeps));
   return stricmp(cf1.c_str(), cf2.c_str());
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cStrTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cStrTests);
      CPPUNIT_TEST(TestParseBadArgs);
      CPPUNIT_TEST(TestParseSuccessCases);
      CPPUNIT_TEST(TestFilePathCompare);
      CPPUNIT_TEST(TestFormat);
   CPPUNIT_TEST_SUITE_END();

   void TestParseBadArgs();
   void TestParseSuccessCases();
   void TestFilePathCompare();
   void TestFormat();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cStrTests);

////////////////////////////////////////

void cStrTests::TestParseBadArgs()
{
   CPPUNIT_ASSERT(cStr("blah").ParseTuple((double *)NULL, 1) == E_POINTER);
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

////////////////////////////////////////

void cStrTests::TestFilePathCompare()
{
   CPPUNIT_ASSERT(filepathcmp("c:\\p1\\p2\\p3", "c:/p1/p2/p3") == 0);
   CPPUNIT_ASSERT(filepathcmp("C:\\P1\\P2\\P3", "c:/p1/p2/p3") != 0);
   CPPUNIT_ASSERT(filepathicmp("C:\\P1\\P2\\P3", "c:/p1/p2/p3") == 0);
   CPPUNIT_ASSERT(filepathcmp("c:\\p1\\p2\\p3", "c:\\p4\\p5\\p6") < 0);
}

////////////////////////////////////////

void cStrTests::TestFormat()
{
   cStr temp;
   CPPUNIT_ASSERT(temp.Format("%c", 'X') > 0);
   CPPUNIT_ASSERT(temp.compare("X") == 0);
   CPPUNIT_ASSERT(temp.Format("%d", 1000) > 0);
   CPPUNIT_ASSERT(temp.compare("1000") == 0);
   CPPUNIT_ASSERT(temp.Format("%i", 1000) > 0);
   CPPUNIT_ASSERT(temp.compare("1000") == 0);
   CPPUNIT_ASSERT(temp.Format("%o", 1000) > 0);
   CPPUNIT_ASSERT(temp.compare("1750") == 0);
   CPPUNIT_ASSERT(temp.Format("%u", 1000) > 0);
   CPPUNIT_ASSERT(temp.compare("1000") == 0);
   CPPUNIT_ASSERT(temp.Format("%u", -1000) > 0);
   CPPUNIT_ASSERT(temp.compare("4294966296") == 0);
   CPPUNIT_ASSERT(temp.Format("%x", -1000) > 0);
   CPPUNIT_ASSERT(temp.compare("fffffc18") == 0);
   CPPUNIT_ASSERT(temp.Format("%X", -1000) > 0);
   CPPUNIT_ASSERT(temp.compare("FFFFFC18") == 0);
   CPPUNIT_ASSERT(temp.Format("%e", 9999.0 * 3.14159) > 0);
   CPPUNIT_ASSERT(temp.compare("3.141276e+004") == 0);
   CPPUNIT_ASSERT(temp.Format("%E", 9999.0 * 3.14159) > 0);
   CPPUNIT_ASSERT(temp.compare("3.141276E+004") == 0);

   CPPUNIT_ASSERT(temp.Format("abcd%shijkl", "efg") > 0);
   CPPUNIT_ASSERT(temp.compare("abcdefghijkl") == 0);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
