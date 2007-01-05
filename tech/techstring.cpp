///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/techstring.h"
#include "tech/techmath.h"
#include "tech/comtools.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cfloat>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <locale>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

static size_t UTF8Encode(uint32 wideChar, char * pDest, size_t maxDest)
{
   if (wideChar < 0x80)
   {
      if (pDest != NULL && maxDest >= 1)
      {
         *pDest = (wideChar & 0xFF);
      }
      return 1;
   }
   else if (wideChar < 0x800)
   {
      if (pDest != NULL && maxDest >= 2)
      {
         *pDest++ = (0xC0 | wideChar >> 6);
         *pDest++ = (0x80 | wideChar & 0x3F);
      }
      return 2;
   }
   else if (wideChar < 0x10000)
   {
      if (pDest != NULL && maxDest >= 3)
      {
         *pDest++ = (0xE0 | wideChar >> 12);
         *pDest++ = (0x80 | wideChar >> 6 & 0x3F);
         *pDest++ = (0x80 | wideChar & 0x3F);
      }
      return 3;
   }
   else if (wideChar < 0x200000)
   {
      if (pDest != NULL && maxDest >= 4)
      {
         *pDest++ = (0xF0 | wideChar >> 18);
         *pDest++ = (0x80 | wideChar >> 12 & 0x3F);
         *pDest++ = (0x80 | wideChar >> 6 & 0x3F);
         *pDest++ = (0x80 | wideChar & 0x3F);
      }
      return 4;
   }
   return 0;
}

size_t UTF8Encode(const wchar_t * pszSrc, char * pszDest, size_t maxDest)
{
   if (pszSrc == NULL)
   {
      return 0;
   }

   char * p = pszDest;
   size_t destLen = 0;

   size_t srcLen = wcslen(pszSrc);
   for (uint i = 0; i < srcLen; i++)
   {
      size_t available = (maxDest > destLen) ? maxDest - destLen : 0;
      size_t required = UTF8Encode(pszSrc[i], p, available);
      p += required;
      destLen += required;
   }

   if (pszDest != NULL && maxDest > 0)
   {
      pszDest[Min(maxDest - 1, destLen)] = 0;
   }

   return destLen;
}


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
   int i = strtol(formatOptions.c_str(), NULL, 10);
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

AssertAtCompileTime(sizeof(int) == sizeof(uint));

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

   AssertMsg(formatLenEst < (4 * 1024), _T("Format string length estimate is very large"));

   return formatLenEst;
}


///////////////////////////////////////////////////////////////////////////////
// Based on public domain code from an old C/C++ User's Journal article

bool WildCardMatch(const tChar * pszPattern, const tChar * pszString)
{
   int i;
   bool star;

new_segment:
   star = false;
   if (*pszPattern == _T('*'))
   {
      star = true;
      do { pszPattern = _tcsinc(pszPattern); } while (*pszPattern == _T('*'));
   }

   for (;;)
   {
      for (i = 0; pszPattern[i] && (pszPattern[i] != _T('*')); i++)
      {
         if (pszString[i] != pszPattern[i])
         {
            if (!pszString[i])
               return false;
            if ((pszPattern[i] == _T('?')) && (pszString[i] != _T('.')))
               continue;
            if (!star)
               return false;
            pszString = _tcsinc(pszString);
         }
      }
      if (pszPattern[i] == _T('*'))
      {
         for (int j = 0; j < i; j++)
         {
            pszString = _tcsinc(pszString);
            pszPattern = _tcsinc(pszPattern);
         }
         goto new_segment;
      }
      else
      {
         if (!pszString[i])
            return true;
         if (i && pszPattern[i - 1] == _T('*'))
            return true;
         if (!star)
            return false;
         pszString = _tcsinc(pszString);
      }
   }

   return false;
}


///////////////////////////////////////////////////////////////////////////////

std::string cToken<std::string, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   size_t s = wcstombs(NULL, pszToken, 0);
   if (s == 0)
   {
      return std::string();
   }
   char * pszTemp = reinterpret_cast<char*>(alloca(s * sizeof(char)));
   wcstombs(pszTemp, pszToken, s);
   return std::string(pszTemp);
#else
   return std::string(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

double cToken<double, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

float cToken<float, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////

int cToken<int, tChar>::Token(const tChar * pszToken)
{
#ifdef __GNUC__
   return strtol(pszToken, NULL, 10);
#else
   return _ttoi(pszToken);
#endif
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

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////

TEST(TokenizerBadArgs)
{
   cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
   CHECK(strTok.Tokenize(NULL) == E_POINTER);
   CHECK(strTok.Tokenize("[1,2,3") == E_INVALIDARG); // missing end bracket
}

////////////////////////////////////////

TEST(TokenizerSuccessCases)
{
   {
      cTokenizer<std::string, std::vector<std::string>, std::string::value_type> strTok;
      CHECK(strTok.Tokenize("[]") == 0);
      CHECK(strTok.Tokenize("{  }") == 0);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("[1,2,3,4]") == 4);
      CHECK(dblTok.m_tokens[0] == 1);
      CHECK(dblTok.m_tokens[1] == 2);
      CHECK(dblTok.m_tokens[2] == 3);
      CHECK(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("< 10 ; 32.2 ; 48.8 >") == 3);
      CHECK(dblTok.m_tokens[0] == 10.0);
      CHECK(dblTok.m_tokens[1] == 32.2);
      CHECK(dblTok.m_tokens[2] == 48.8);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("(1,2,3,4)") == 4);
      CHECK(dblTok.m_tokens[0] == 1);
      CHECK(dblTok.m_tokens[1] == 2);
      CHECK(dblTok.m_tokens[2] == 3);
      CHECK(dblTok.m_tokens[3] == 4);
   }

   {
      cTokenizer<double> dblTok;
      CHECK(dblTok.Tokenize("1000,2000,3000,4000") == 4);
      CHECK(dblTok.m_tokens[0] == 1000);
      CHECK(dblTok.m_tokens[1] == 2000);
      CHECK(dblTok.m_tokens[2] == 3000);
      CHECK(dblTok.m_tokens[3] == 4000);
   }
}

////////////////////////////////////////

TEST(Sprintf)
{
   cStr temp;
   CHECK(Sprintf(&temp, "%c", 'X').compare("X") == 0);
   CHECK(Sprintf(&temp, "%d", 1000).compare("1000") == 0);
   CHECK(Sprintf(&temp, "%i", 1000).compare("1000") == 0);
   CHECK(Sprintf(&temp, "%o", 1000).compare("1750") == 0);
   CHECK(Sprintf(&temp, "%u", 1000).compare("1000") == 0);
   CHECK(Sprintf(&temp, "%u", -1000).compare("4294966296") == 0);
   CHECK(Sprintf(&temp, "%x", -1000).compare("fffffc18") == 0);
   CHECK(Sprintf(&temp, "%X", -1000).compare("FFFFFC18") == 0);
   CHECK(Sprintf(&temp, "%e", 9999.0 * 3.14159).compare("3.141276e+004") == 0);
   CHECK(Sprintf(&temp, "%E", 9999.0 * 3.14159).compare("3.141276E+004") == 0);
   CHECK(Sprintf(&temp, "abcd%shijkl", "efg").compare("abcdefghijkl") == 0);
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

TEST(DoSprintfLengthEst)
{
   static const tChar szSample[] = "sample string";
   CHECK(::DoSprintfLengthTest("simple"));
   CHECK(::DoSprintfLengthTest("with integer: %d", INT_MAX));
   CHECK(::DoSprintfLengthTest("with float: %f", DBL_MAX));
   CHECK(::DoSprintfLengthTest("with string: %s", szSample));
   CHECK(::DoSprintfLengthTest("%x %d %s (multiple)", UINT_MAX, INT_MIN, szSample));
   CHECK(::DoSprintfLengthTest("%% escaped percents %%%%%%"));
   CHECK(::DoSprintfLengthTest("hex with specific width %08X", UINT_MAX / 22));
   CHECK(::DoSprintfLengthTest("pointer %p", NULL));
}

#endif // _MSC_VER >= 1300

////////////////////////////////////////

TEST(WildCardMatch)
{
   CHECK(WildCardMatch("AB", "AB"));
   CHECK(!WildCardMatch("AB", "A"));
   CHECK(WildCardMatch("A?", "AB"));
   CHECK(WildCardMatch("A?", "AC"));
   CHECK(WildCardMatch("A*", "Abcd"));
   CHECK(WildCardMatch("A**", "Abcd"));
   CHECK(WildCardMatch("*A", "abcdA"));
   CHECK(WildCardMatch("**A", "abcdA"));
   CHECK(WildCardMatch("*.xml", "foo.xml"));
   CHECK(WildCardMatch("f*.xml", "foo.xml"));
   CHECK(WildCardMatch("f*.xml", "fog.xml"));
   CHECK(WildCardMatch("f*.xml", "fubar.xml"));
   CHECK(!WildCardMatch("f*.xml", "bar.xml"));
   CHECK(WildCardMatch("w*d.test", "wildcard.test"));
   CHECK(WildCardMatch("w*d.test", "word.test"));
   CHECK(WildCardMatch("w*d.t*t", "word.txt"));
}

////////////////////////////////////////

TEST(GUIDToString)
{
   // {B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}
   static const GUID kTestGuidA = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};
   static const GUID kTestGuidADupe = {0xb40b6831, 0xfcb2, 0x4082, {0xae, 0x7, 0x61, 0xa7, 0xfc, 0x4d, 0x3a, 0xeb}};

   // {DED26BAE-E83F-4c59-8B95-A309311B15A9}
   static const GUID kTestGuidB = {0xded26bae, 0xe83f, 0x4c59, {0x8b, 0x95, 0xa3, 0x9, 0x31, 0x1b, 0x15, 0xa9}};

   cStr guid;
   GUIDToString(kTestGuidA, &guid);
   CHECK(guid.compare("{B40B6831-FCB2-4082-AE07-61A7FC4D3AEB}") == 0);

   CHECK(CTIsEqualGUID(kTestGuidA, kTestGuidADupe));
   CHECK(!CTIsEqualGUID(kTestGuidA, kTestGuidB));
}

////////////////////////////////////////

// This unit test requires support for Unicode string literals
#if _MSC_VER >= 1400
TEST(UTF8Encode)
{
   // Test strings from the UTF-8 RFC, http://www.ietf.org/rfc/rfc3629.txt

   {
      char szActual[100];
      CHECK(UTF8Encode(L"A\u2262\u0391.", szActual, _countof(szActual)));
      byte expected[] = { 0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E };
      CHECK(memcmp(szActual, expected, sizeof(expected)) == 0);
   }

   {
      char szActual[100];
      CHECK(UTF8Encode(L"\uD55C\uAD6D\uC5B4", szActual, _countof(szActual)));
      byte expected[] = { 0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4 };
      CHECK(memcmp(szActual, expected, sizeof(expected)) == 0);
   }

   {
      char szActual[100];
      CHECK(UTF8Encode(L"\u65E5\u672C\u8A9E", szActual, _countof(szActual)));
      byte expected[] = { 0xE6, 0x97, 0xA5, 0xE6, 0x9C, 0xAC, 0xE8, 0xAA, 0x9E };
      CHECK(memcmp(szActual, expected, sizeof(expected)) == 0);
   }

   //{
   //   char szActual[100];
   //   CHECK(UTF8Encode(L"\u233B4", szActual, _countof(szActual)));
   //   byte expected[] = { 0xA3, 0x8E, 0xB4 };
   //   CHECK(memcmp(szActual, expected, sizeof(expected)) == 0);
   //}
}
#endif

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
