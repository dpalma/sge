///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiparse.h"
#include "guiapi.h"
#include "guistrings.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

const tChar * SkipSpaceFwd(const tChar * psz)
{
   Assert(psz != NULL);
   while (_istspace(*psz))
      psz++;
   return psz;
}

const tChar * SkipSpaceBack(const tChar * psz)
{
   Assert(psz != NULL);
   while (_istspace(*(psz - 1)))
      psz--;
   return psz;
}

///////////////////////////////////////////////////////////////////////////////

const tChar * SkipSingleComment(const tChar * psz)
{
   Assert(psz != NULL);

   if (*psz == _T('/'))
   {
      if (*(psz+1) == _T('*'))
      {
         const tChar * pszStop = _tcsstr(psz+2, _T("*/"));
         if (pszStop != NULL)
         {
            psz = pszStop + 2;
         }
         else
         {
            psz = psz + _tcslen(psz);
         }
      }
      else if (*(psz+1) == _T('/'))
      {
         const tChar * pszStop = _tcschr(psz+2, _T('\n'));
         if (pszStop != NULL)
         {
            psz = pszStop + 1;
         }
         else
         {
            psz = psz + _tcslen(psz);
         }
      }
   }

   return psz;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIParseStyleDimension(const tChar * psz, int * pDimension, tGUIDimensionSpec * pSpec)
{
   if (psz == NULL || pDimension == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   int dimension = -1;
   tChar szSpec[32] = {0};

   // The 32 must match the size of szSpec. Not using "%*s" and passing in the buffer
   // length because then scanf won't recognize a single percent character as a valid
   // value for the string. That is, this case will not parse as desired: "25%".
   int nParsed = _stscanf(psz, _T("%d%32s"), &dimension, szSpec);
   if (nParsed == 2)
   {
      if (_tcscmp(szSpec, _T("%")) == 0)
      {
         *pDimension = dimension;
         *pSpec = kGUIDimensionPercent;
         return S_OK;
      }
      else if (_tcsicmp(szSpec, _T("px")) == 0)
      {
         *pDimension = dimension;
         *pSpec = kGUIDimensionPixels;
         return S_OK;
      }
   }
   else if (nParsed == 1)
   {
      *pDimension = dimension;
      *pSpec = kGUIDimensionPixels;
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIParseColor(const tChar * pszColor, tGUIColor * pColor)
{
   if (pszColor == NULL || pColor == NULL)
   {
      return E_POINTER;
   }
   static const struct sNamedColor
   {
      sNamedColor(const tChar * p, const tGUIColor & c) : pszName(p), color(c) {}
      const tChar * pszName;
      tGUIColor color;
   }
   g_namedColorTable[] =
   {
      sNamedColor(kValueColorBlack,       tGUIColor::Black),
      sNamedColor(kValueColorRed,         tGUIColor::Red),
      sNamedColor(kValueColorGreen,       tGUIColor::Green),
      sNamedColor(kValueColorYellow,      tGUIColor::Yellow),
      sNamedColor(kValueColorBlue,        tGUIColor::Blue),
      sNamedColor(kValueColorMagenta,     tGUIColor::Magenta),
      sNamedColor(kValueColorCyan,        tGUIColor::Cyan),
      sNamedColor(kValueColorDarkGray,    tGUIColor::DarkGray),
      sNamedColor(kValueColorGray,        tGUIColor::Gray),
      sNamedColor(kValueColorLightGray,   tGUIColor::LightGray),
      sNamedColor(kValueColorWhite,       tGUIColor::White),
   };

   float rgba[4];
   int parseResult = cStr(pszColor).ParseTuple(rgba, _countof(rgba));
   if (parseResult == 3)
   {
      *pColor = tGUIColor(rgba[0],rgba[1],rgba[2]);
      return S_OK;
   }
   else if (parseResult == 4)
   {
      *pColor = tGUIColor(rgba[0],rgba[1],rgba[2],rgba[3]);
      return S_OK;
   }
   else
   {
      for (int i = 0; i < _countof(g_namedColorTable); ++i)
      {
         if (_tcsicmp(g_namedColorTable[i].pszName, pszColor) == 0)
         {
            *pColor = g_namedColorTable[i].color;
            return S_OK;
         }
      }
   }
   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIParseBool(const tChar * pszBool, bool * pBool)
{
   if (pszBool == NULL || pBool == NULL)
   {
      return E_POINTER;
   }

   // empty string means false
   if (*pszBool == 0)
   {
      *pBool = false;
      return S_OK;
   }

   int n = 0;
   if (_stscanf(pszBool, _T("%d"), &n) == 1)
   {
      *pBool = (n != 0);
      return S_OK;
   }

   if (_tcsicmp(pszBool, _T("true")) == 0)
   {
      *pBool = true;
      return S_OK;
   }
   else if (_tcsicmp(pszBool, _T("false")) == 0)
   {
      *pBool = false;
      return S_OK;
   }

   return E_INVALIDARG;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUIParseTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIParseTests);
      CPPUNIT_TEST(TestParseDimension);
      CPPUNIT_TEST(TestParseColor);
      CPPUNIT_TEST(TestParseBool);
   CPPUNIT_TEST_SUITE_END();

   void TestParseDimension();
   void TestParseColor();
   void TestParseBool();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIParseTests);

///////////////////////////////////////

void cGUIParseTests::TestParseDimension()
{
   int dim;
   tGUIDimensionSpec spec;

   CPPUNIT_ASSERT(GUIParseStyleDimension("150", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 150) && (spec == kGUIDimensionPixels));
   CPPUNIT_ASSERT(GUIParseStyleDimension("250px", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 250) && (spec == kGUIDimensionPixels));
   CPPUNIT_ASSERT(GUIParseStyleDimension("350 px", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 350) && (spec == kGUIDimensionPixels));
   CPPUNIT_ASSERT(GUIParseStyleDimension("25%", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 25) && (spec == kGUIDimensionPercent));
   CPPUNIT_ASSERT(GUIParseStyleDimension("55  %", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 55) && (spec == kGUIDimensionPercent));

   // try a value too big to fit in a 32-bit int--should get truncated
   CPPUNIT_ASSERT(GUIParseStyleDimension("3232323232323232323232323232323232", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 347068064) && (spec == kGUIDimensionPixels));

   // try a completely bogus value
   CPPUNIT_ASSERT(FAILED(GUIParseStyleDimension("there is no way to parse this as a style dimension", &dim, &spec)));

   // try a value with a small enough string but still bogus
   CPPUNIT_ASSERT(FAILED(GUIParseStyleDimension("still no way", &dim, &spec)));

   // only partially bogus--valid integer but invalid string after it
   CPPUNIT_ASSERT(FAILED(GUIParseStyleDimension("99 units", &dim, &spec)));
}

///////////////////////////////////////

void cGUIParseTests::TestParseColor()
{
   tGUIColor color;
   CPPUNIT_ASSERT(GUIParseColor("black", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Black);
   CPPUNIT_ASSERT(GUIParseColor("red", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Red);
   CPPUNIT_ASSERT(GUIParseColor("green", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Green);
   CPPUNIT_ASSERT(GUIParseColor("yellow", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Yellow);
   CPPUNIT_ASSERT(GUIParseColor("blue", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Blue);
   CPPUNIT_ASSERT(GUIParseColor("magenta", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Magenta);
   CPPUNIT_ASSERT(GUIParseColor("cyan", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Cyan);
   CPPUNIT_ASSERT(GUIParseColor("darkgray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::DarkGray);
   CPPUNIT_ASSERT(GUIParseColor("gray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Gray);
   CPPUNIT_ASSERT(GUIParseColor("lightgray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::LightGray);
   CPPUNIT_ASSERT(GUIParseColor("white", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::White);
}

///////////////////////////////////////

void cGUIParseTests::TestParseBool()
{
   bool b;
   CPPUNIT_ASSERT(GUIParseBool("1", NULL) == E_POINTER);
   CPPUNIT_ASSERT(GUIParseBool(NULL, &b) == E_POINTER);
   CPPUNIT_ASSERT(GUIParseBool("", &b) == S_OK);
   CPPUNIT_ASSERT(!b);
   CPPUNIT_ASSERT(GUIParseBool("1", &b) == S_OK);
   CPPUNIT_ASSERT(b);
   CPPUNIT_ASSERT(GUIParseBool("0", &b) == S_OK);
   CPPUNIT_ASSERT(!b);
   CPPUNIT_ASSERT(GUIParseBool("true", &b) == S_OK);
   CPPUNIT_ASSERT(b);
   CPPUNIT_ASSERT(GUIParseBool("tRuE", &b) == S_OK);
   CPPUNIT_ASSERT(b);
   CPPUNIT_ASSERT(GUIParseBool("false", &b) == S_OK);
   CPPUNIT_ASSERT(!b);
   CPPUNIT_ASSERT(GUIParseBool("fAlSe", &b) == S_OK);
   CPPUNIT_ASSERT(!b);
   CPPUNIT_ASSERT(GUIParseBool("arbitrary string", &b) == E_INVALIDARG);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
