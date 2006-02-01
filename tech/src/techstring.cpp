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
//
// CLASS: cStr
//

const cStr cStr::gm_whitespace(_T(" \t\r\n"));

///////////////////////////////////////

int cStr::ToInt() const
{
#ifdef __GNUC__
   return Round(strtod(Get(), NULL));
#else
   return _ttoi(Get());
#endif
}

///////////////////////////////////////

float cStr::ToFloat() const
{
#ifdef _UNICODE
   return static_cast<float>(_wtof(Get()));
#else
   return static_cast<float>(atof(Get()));
#endif
}

///////////////////////////////////////

double cStr::ToDouble() const
{
#ifdef _UNICODE
   return _wtof(Get());
#else
   return atof(Get());
#endif
}

///////////////////////////////////////

int cStr::ParseTuple(std::vector<cStr> * pStrings, const tChar * pszDelims /*=NULL*/) const
{
   if (pStrings == NULL)
   {
      return E_POINTER;
   }

   if (pszDelims == NULL)
   {
      pszDelims = _T(",;");
   }

   pStrings->clear();

   const tChar * psz = Get();
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
         pszStop = _tcsdec(Get(), pszStop);
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
         pStrings->push_back(cStr(psz, len));
         nParsed++;
      }

      psz = _tcsinc(psz2);
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

   std::vector<cStr> strings;
   int result = ParseTuple(&strings);
   if (result > 0)
   {
      int count = 0;
      std::vector<cStr>::iterator iter = strings.begin();
      std::vector<cStr>::iterator end = strings.end();
      for (; (iter != end) && (count < nMaxDoubles); iter++, count++)
      {
         *pDoubles++ = iter->ToDouble();
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

static int FormatOptionsLengthEstimate(const cStr & formatOptions, tChar type)
{
   if (formatOptions.empty())
   {
      return 0;
   }

   int i = formatOptions.ToInt();
   if (i > 0)
   {
      return i;
   }

   // TODO: there are more cases to handle
   return 0;
}

AssertOnce(sizeof(int) == sizeof(uint));

static int FormatLengthEstimate(const tChar * pszFormat, va_list args)
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
               formatLenEst += FormatOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
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
               formatLenEst += FormatOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
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
               formatLenEst += FormatOptionsLengthEstimate(formatOptions.c_str(), pszFormat[i]);
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

///////////////////////////////////////

int CDECL cStr::Format(const tChar * pszFormat, ...)
{
   va_list args;
   va_start(args, pszFormat);
   int length = FormatLengthEstimate(pszFormat, args) + 1; // plus one for null terminator
   tChar * pszTemp = reinterpret_cast<tChar*>(alloca(length * sizeof(tChar)));
   int result = _vsntprintf(pszTemp, length, pszFormat, args);
   va_end(args);
   assign(pszTemp);
   return result;
}

///////////////////////////////////////////////////////////////////////////////

const cStr & GUIDToString(REFGUID guid, cStr * pStr)
{
   if (pStr != NULL)
   {
      Verify(pStr->Format(_T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
         guid.Data1, guid.Data2, guid.Data3,
         guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
         guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]) > 0);
      return *pStr;
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
   CPPUNIT_TEST_SUITE(cStrTests);
      CPPUNIT_TEST(TestParseBadArgs);
      CPPUNIT_TEST(TestParseSuccessCases);
      CPPUNIT_TEST(TestFilePathCompare);
      CPPUNIT_TEST(TestFormat);
#if _MSC_VER >= 1300
      CPPUNIT_TEST(TestFormatLength);
#endif
      CPPUNIT_TEST(TestGUIDToString);
   CPPUNIT_TEST_SUITE_END();

   void TestParseBadArgs();
   void TestParseSuccessCases();
   void TestFilePathCompare();
   void TestFormat();
#if _MSC_VER >= 1300
   void TestFormatLength();
#endif
   void TestGUIDToString();
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

////////////////////////////////////////

#if _MSC_VER >= 1300

static bool CDECL DoFormatLengthTest(const tChar * pszFormat, ...)
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
      estimate = FormatLengthEstimate(pszFormat, args);
      va_end(args);
   }
   // Over-estimating is OK, therefore the ">="
   return (estimate >= actual);
}

////////////////////////////////////////

void cStrTests::TestFormatLength()
{
   static const tChar szSample[] = "sample string";
   CPPUNIT_ASSERT(::DoFormatLengthTest("simple"));
   CPPUNIT_ASSERT(::DoFormatLengthTest("with integer: %d", INT_MAX));
   CPPUNIT_ASSERT(::DoFormatLengthTest("with float: %f", DBL_MAX));
   CPPUNIT_ASSERT(::DoFormatLengthTest("with string: %s", szSample));
   CPPUNIT_ASSERT(::DoFormatLengthTest("%x %d %s (multiple)", UINT_MAX, INT_MIN, szSample));
   CPPUNIT_ASSERT(::DoFormatLengthTest("%% escaped percents %%%%%%"));
   CPPUNIT_ASSERT(::DoFormatLengthTest("hex with specific width %08X", UINT_MAX / 22));
   CPPUNIT_ASSERT(::DoFormatLengthTest("pointer %p", NULL));
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
