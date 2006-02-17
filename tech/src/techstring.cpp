///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techstring.h"
#include "techmath.h"
#include "comtools.h"

#include <cfloat>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

int ParseTuple(const tChar * pszIn, const tChar * pszDelims,
               tParseTupleCallbackFn pfnCallback, uint_ptr userData)
{
   if (pszIn == NULL || pfnCallback == NULL)
   {
      return E_POINTER;
   }

   if (pszDelims == NULL)
   {
      pszDelims = _T(",;");
   }

   const tChar * psz = pszIn;
   const tChar * pszStop = psz + _tcslen(psz);

   // the open and close bracket strings must "line up"
   // for example, '<' and '>' both appear at index zero
   static const tChar szOpenBrackets[]  = _T("<([{");
   static const tChar szCloseBrackets[] = _T(">)]}");

   const tChar * pszOpenBracket = _tcschr(szOpenBrackets, *psz);
   if (pszOpenBracket != NULL)
   {
      if (*(pszStop - 1) == szCloseBrackets[pszOpenBracket - szOpenBrackets])
      {
         psz = _tcsinc(psz);
         pszStop = _tcsdec(pszIn, pszStop);
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
         psz = _tcsinc(psz);
      }

      const tChar * psz2 = _tcspbrk(psz, pszDelims);
      if (psz2 == NULL)
      {
         psz2 = pszStop;
      }

      int len = psz2 - psz;
      if (len > 0)
      {
         cStr temp(psz, len);
         (*pfnCallback)(temp.c_str(), userData);
         nParsed++;
      }

      psz = _tcsinc(psz2);
   }

   return nParsed;
}


///////////////////////////////////////////////////////////////////////////////

static int SprintfOptionsLengthEstimate(const cStr & formatOptions, tChar type)
{
   if (formatOptions.empty())
   {
      return 0;
   }

#ifdef __GNUC__
   int i = Round(strtod(formatOptions.c_str()));
#else
   int i = _ttoi(formatOptions.c_str());
#endif

   if (i > 0)
   {
      return i;
   }

   // TODO: there are more cases to handle
   return 0;
}

///////////////////////////////////////////////////////////////////////////////

AssertOnce(sizeof(int) == sizeof(uint));

int SprintfLengthEstimate(const tChar * pszFormat, va_list args)
{
   const uint l = _tcslen(pszFormat);

   int formatLenEst = 0;
   tChar last = 0;
   bool bInFormatField = false;
   cStr formatOptions; // stuff between percent and type-specifying character (e.g., 08 in %08x)

   for (uint i = 0; i < l; i++)
   {
      switch (pszFormat[i])
      {
         case '%':
         {
            if (last == '%')
            {
               // Escaped percent sign ("%%")
               formatLenEst += 1;
               Assert(bInFormatField);
               bInFormatField = false;
            }
            else
            {
               Assert(!bInFormatField);
               bInFormatField = true;
               formatOptions.erase();
            }
            break;
         }

         case 'p':
         {
            if (bInFormatField)
            {
               bInFormatField = false;
               formatLenEst += (sizeof(void*) * 2);
            }
            else
            {
               formatLenEst += 1;
            }
            break;
         }

         case 'c':
         {
            if (bInFormatField)
            {
               bInFormatField = false;
            }
            // Add one regardless of whether this is the character 'c'
            // or the format field "%c"--both will have length of one.
            formatLenEst += 1;
            break;
         }

         case 'd':
         case 'i':
         case 'o':
         case 'u':
         case 'x':
         case 'X':
         {
            if (bInFormatField)
            {
               int intValue = va_arg(args, int);
               bInFormatField = false;
               // The #bits is a safe over-estimate of the max #digits
               formatLenEst += sizeof(int) * CHAR_BIT;
               formatLenEst += SprintfOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
            }
            else
            {
               formatLenEst += 1;
            }
            break;
         }

         case 'e':
         case 'E':
         case 'f':
         case 'g':
         case 'G':
         {
            if (bInFormatField)
            {
               double doubleValue = va_arg(args, double);
               bInFormatField = false;
               formatLenEst += (DBL_MANT_DIG * CHAR_BIT) + 1 + DBL_DIG;
               formatLenEst += SprintfOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
            }
            else
            {
               formatLenEst += 1;
            }
            break;
         }

         case 's':
         {
            if (bInFormatField)
            {
               const tChar * pszValue = va_arg(args, const tChar *);
               bInFormatField = false;
               formatLenEst += _tcslen(pszValue);
               formatLenEst += SprintfOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
            }
            else
            {
               formatLenEst += 1;
            }
            break;
         }

         default:
         {
            if (bInFormatField)
            {
               formatOptions += pszFormat[i];
            }
            else
            {
               formatLenEst += 1;
            }
            break;
         }
      }

      if (last == '%' && pszFormat[i] == '%')
      {
         // Don't interpret the next character as a format field
         last = 0;
      }
      else
      {
         last = pszFormat[i];
      }
   }

   AssertMsg(formatLenEst < (4 * 1024), "Format string length estimate is very large");

   return formatLenEst;
}

///////////////////////////////////////////////////////////////////////////////

const cStr & GUIDToString(REFGUID guid, cStr * pStr)
{
   if (pStr != NULL)
   {
      return Sprintf(pStr, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
         guid.Data1, guid.Data2, guid.Data3,
         guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
         guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
   }
   static const cStr empty;
   return empty;
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

static const cStr g_fileSeps(_T("\\/"));

int filepathcmp(const cStr & f1, const cStr & f2)
{
   cStr cf1(FilteredCopy(f1, g_fileSeps));
   cStr cf2(FilteredCopy(f2, g_fileSeps));
   return _tcscmp(cf1.c_str(), cf2.c_str());
}

///////////////////////////////////////////////////////////////////////////////

int filepathicmp(const cStr & f1, const cStr & f2)
{
   cStr cf1(FilteredCopy(f1, g_fileSeps));
   cStr cf2(FilteredCopy(f2, g_fileSeps));
   return _tcsicmp(cf1.c_str(), cf2.c_str());
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cStrTests : public CppUnit::TestCase
{
   void TestParseBadArgs();
   void TestParseSuccessCases();
   void TestFilePathCompare();
   void TestSprintf();
#if _MSC_VER >= 1300
   void TestSprintfLengthEst();
#endif
   void TestGUIDToString();

   CPPUNIT_TEST_SUITE(cStrTests);
      CPPUNIT_TEST(TestParseBadArgs);
      CPPUNIT_TEST(TestParseSuccessCases);
      CPPUNIT_TEST(TestFilePathCompare);
      CPPUNIT_TEST(TestSprintf);
#if _MSC_VER >= 1300
      CPPUNIT_TEST(TestSprintfLengthEst);
#endif
      CPPUNIT_TEST(TestGUIDToString);
   CPPUNIT_TEST_SUITE_END();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cStrTests);

////////////////////////////////////////

void cStrTests::TestParseBadArgs()
{
   cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
   CPPUNIT_ASSERT(strTok.Tokenize(NULL) == E_POINTER);
   CPPUNIT_ASSERT(strTok.Tokenize("[1,2,3") == E_INVALIDARG); // missing end bracket
}

////////////////////////////////////////

void cStrTests::TestParseSuccessCases()
{
   {
      cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
      CPPUNIT_ASSERT(strTok.Tokenize("[]") == 0);
      CPPUNIT_ASSERT(strTok.Tokenize("{  }") == 0);
   }

   {
      cTokenizer<double> dblTok;
      CPPUNIT_ASSERT(dblTok.Tokenize("[1,2,3,4]") == 4);
      CPPUNIT_ASSERT(dblTok.m_tokens[0] == 1);
      CPPUNIT_ASSERT(dblTok.m_tokens[1] == 2);
      CPPUNIT_ASSERT(dblTok.m_tokens[2] == 3);
      CPPUNIT_ASSERT(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CPPUNIT_ASSERT(dblTok.Tokenize("< 10 ; 32.2 ; 48.8 >") == 3);
      CPPUNIT_ASSERT(dblTok.m_tokens[0] == 10);
      CPPUNIT_ASSERT(dblTok.m_tokens[1] == 32.2);
      CPPUNIT_ASSERT(dblTok.m_tokens[2] == 48.8);
   }

   {
      cTokenizer<double> dblTok;
      CPPUNIT_ASSERT(dblTok.Tokenize("(1,2,3,4)") == 4);
      CPPUNIT_ASSERT(dblTok.m_tokens[0] == 1);
      CPPUNIT_ASSERT(dblTok.m_tokens[1] == 2);
      CPPUNIT_ASSERT(dblTok.m_tokens[2] == 3);
      CPPUNIT_ASSERT(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CPPUNIT_ASSERT(dblTok.Tokenize("1000,2000,3000,4000") == 4);
      CPPUNIT_ASSERT(dblTok.m_tokens[0] == 1000);
      CPPUNIT_ASSERT(dblTok.m_tokens[1] == 2000);
      CPPUNIT_ASSERT(dblTok.m_tokens[2] == 3000);
      CPPUNIT_ASSERT(dblTok.m_tokens[3] == 4000);
   }
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

void cStrTests::TestSprintf()
{
   cStr temp;
   CPPUNIT_ASSERT(Sprintf(&temp, "%c", 'X').compare("X") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%d", 1000).compare("1000") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%i", 1000).compare("1000") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%o", 1000).compare("1750") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%u", 1000).compare("1000") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%u", -1000).compare("4294966296") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%x", -1000).compare("fffffc18") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%X", -1000).compare("FFFFFC18") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%e", 9999.0 * 3.14159).compare("3.141276e+004") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "%E", 9999.0 * 3.14159).compare("3.141276E+004") == 0);
   CPPUNIT_ASSERT(Sprintf(&temp, "abcd%shijkl", "efg").compare("abcdefghijkl") == 0);
}

////////////////////////////////////////

#if _MSC_VER >= 1300

static bool CDECL DoSprintfLengthTest(const tChar * pszFormat, ...)
{
   int actual = INT_MIN, estimate = INT_MAX;
   va_list args;
   {
      va_start(args, pszFormat);
      actual = _vscprintf(pszFormat, args); // TODO: this is MS-specific
      va_end(args);
   }
   {
      va_start(args, pszFormat);
      estimate = SprintfLengthEstimate(pszFormat, args);
      va_end(args);
   }
   // Over-estimating is OK, therefore the ">="
   return (estimate >= actual);
}

////////////////////////////////////////

void cStrTests::TestSprintfLengthEst()
{
   static const tChar szSample[] = "sample string";
   CPPUNIT_ASSERT(::DoSprintfLengthTest("simple"));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("with integer: %d", INT_MAX));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("with float: %f", DBL_MAX));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("with string: %s", szSample));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("%x %d %s (multiple)", UINT_MAX, INT_MIN, szSample));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("%% escaped percents %%%%%%"));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("hex with specific width %08X", UINT_MAX / 22));
   CPPUNIT_ASSERT(::DoSprintfLengthTest("pointer %p", NULL));
}

#endif // _MSC_VER >= 1300

////////////////////////////////////////

void cStrTests::TestGUIDToString()
{
   // {B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}
   static const GUID kTestGuidA = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};
   static const GUID kTestGuidADupe = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};

   // {DED26BAE-E83F-4c59-8B95-A309311B15A9}
   static const GUID kTestGuidB = {0xded26bae, 0xe83f, 0x4c59, {0x8b, 0x95, 0xa3, 0x9, 0x31, 0x1b, 0x15, 0xa9}};

   cStr guid;
   GUIDToString(kTestGuidA, &guid);
   CPPUNIT_ASSERT(guid.compare("{B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}") == 0);

   CPPUNIT_ASSERT(CTIsEqualGUID(kTestGuidA, kTestGuidADupe));
   CPPUNIT_ASSERT(!CTIsEqualGUID(kTestGuidA, kTestGuidB));
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
