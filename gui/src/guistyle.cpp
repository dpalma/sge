///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"
#include "guielementtools.h"
#include "guiparse.h"
#include "guistrings.h"

#include "color.h"

#include "globalobj.h"

#include <tinyxml.h>

#include <cstring>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static const uint kInvalidUint = ~0u;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

///////////////////////////////////////

cGUIStyle::cGUIStyle()
 : m_alignment(kGUIAlignLeft)
 , m_verticalAlignment(kGUIVertAlignTop)
 , m_pBackground(NULL)
 , m_pForeground(NULL)
 , m_textAlignment(kGUIAlignLeft)
 , m_textVerticalAlignment(kGUIVertAlignTop)
 , m_fontName("")
 , m_fontPointSize(0)
 , m_bFontBold(false)
 , m_bFontItalic(false)
 , m_bFontShadow(false)
 , m_bFontOutline(false)
 , m_width(-1)
 , m_height(-1)
 , m_widthSpec(kInvalidUint)
 , m_heightSpec(kInvalidUint)
{
}

///////////////////////////////////////

cGUIStyle::~cGUIStyle()
{
   delete m_pBackground, m_pBackground = NULL;
   delete m_pForeground, m_pForeground = NULL;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, tGUIString * pValue)
{
   if (pszAttribute == NULL || pValue == NULL)
   {
      return E_POINTER;
   }

   tAttributeMap::iterator f = m_attributeMap.find(pszAttribute);

   if (f == m_attributeMap.end())
   {
      return S_FALSE;
   }

   *pValue = f->second;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, uint * pValue)
{
   if (pszAttribute == NULL || pValue == NULL)
   {
      return E_POINTER;
   }

   tAttributeMap::iterator f = m_attributeMap.find(pszAttribute);

   if (f == m_attributeMap.end())
   {
      return S_FALSE;
   }

   if (sscanf(f->second.c_str(), "%d", pValue) == 1)
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, tGUIColor * pValue)
{
   if (pszAttribute == NULL || pValue == NULL)
   {
      return E_POINTER;
   }

   tAttributeMap::iterator f = m_attributeMap.find(pszAttribute);

   if (f == m_attributeMap.end())
   {
      return S_FALSE;
   }

   if (GUIParseColor(f->second.c_str(), pValue) == S_OK)
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::SetAttribute(const tChar * pszAttribute, const tChar * pszValue)
{
   if (pszAttribute == NULL || pszValue == NULL)
   {
      return E_POINTER;
   }

   if (_tcslen(pszAttribute) == 0)
   {
      return E_INVALIDARG;
   }

   m_attributeMap[pszAttribute] = pszValue;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetAlignment(uint * pAlignment)
{
   if (pAlignment == NULL)
      return E_POINTER;
   *pAlignment = m_alignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetAlignment(uint alignment)
{
   if ((alignment != kGUIAlignLeft)
      && (alignment != kGUIAlignRight)
      && (alignment != kGUIAlignCenter))
   {
      return E_INVALIDARG;
   }
   m_alignment = alignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetVerticalAlignment(uint * pVerticalAlignment)
{
   if (pVerticalAlignment == NULL)
   {
      return E_POINTER;
   }
   *pVerticalAlignment = m_verticalAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetVerticalAlignment(uint verticalAlignment)
{
   if ((verticalAlignment != kGUIVertAlignTop)
      && (verticalAlignment != kGUIVertAlignBottom)
      && (verticalAlignment != kGUIVertAlignCenter))
   {
      return E_INVALIDARG;
   }
   m_verticalAlignment = verticalAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetBackgroundColor(tGUIColor * pBackground)
{
   if (pBackground == NULL)
   {
      return E_POINTER;
   }
   if (m_pBackground == NULL)
   {
      return S_FALSE;
   }
   *pBackground = *m_pBackground;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetBackgroundColor(const tGUIColor & background)
{
   if (m_pBackground != NULL)
   {
      delete m_pBackground;
      m_pBackground = NULL;
   }
   m_pBackground = new tGUIColor;
   if (m_pBackground == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *m_pBackground = background;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetForegroundColor(tGUIColor * pForeground)
{
   if (pForeground == NULL)
   {
      return E_POINTER;
   }
   if (m_pForeground == NULL)
   {
      return S_FALSE;
   }
   *pForeground = *m_pForeground;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetForegroundColor(const tGUIColor & foreground)
{
   if (m_pForeground != NULL)
   {
      delete m_pForeground;
      m_pForeground = NULL;
   }
   m_pForeground = new tGUIColor;
   if (m_pForeground == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *m_pForeground = foreground;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetTextAlignment(uint * pTextAlignment)
{
   if (pTextAlignment == NULL)
      return E_POINTER;
   *pTextAlignment = m_textAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetTextAlignment(uint textAlignment)
{
   if ((textAlignment != kGUIAlignLeft)
      && (textAlignment != kGUIAlignRight)
      && (textAlignment != kGUIAlignCenter))
   {
      return E_INVALIDARG;
   }
   m_textAlignment = textAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetTextVerticalAlignment(uint * pTextVertAlignment)
{
   if (pTextVertAlignment == NULL)
      return E_POINTER;
   *pTextVertAlignment = m_textVerticalAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetTextVerticalAlignment(uint textVertAlignment)
{
   if ((textVertAlignment != kGUIVertAlignTop)
      && (textVertAlignment != kGUIVertAlignBottom)
      && (textVertAlignment != kGUIVertAlignCenter))
   {
      return E_INVALIDARG;
   }
   m_textVerticalAlignment = textVertAlignment;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontName(tGUIString * pFontName)
{
   if (pFontName == NULL)
   {
      return E_POINTER;
   }
   if (m_fontName.length() == 0)
   {
      return S_FALSE;
   }
   *pFontName = m_fontName;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontName(const char * pszFontName)
{
   if (pszFontName == NULL)
   {
      return E_POINTER;
   }
   m_fontName = pszFontName;
   return m_fontName.length() > 0 ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontPointSize(uint * pFontPointSize)
{
   if (pFontPointSize == NULL)
      return E_POINTER;
   *pFontPointSize = m_fontPointSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontPointSize(uint fontPointSize)
{
   if ((fontPointSize < 6) || (fontPointSize > 48))
   {
      DebugMsg1("ERROR: Odd font point size requested: %d\n", fontPointSize);
      return E_INVALIDARG;
   }
   m_fontPointSize = fontPointSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontBold(bool * pB)
{
   if (pB == NULL)
   {
      return E_POINTER;
   }
   *pB = m_bFontBold;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontBold(bool b)
{
   m_bFontBold = b;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontItalic(bool * pB)
{
   if (pB == NULL)
   {
      return E_POINTER;
   }
   *pB = m_bFontItalic;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontItalic(bool b)
{
   m_bFontItalic = b;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontShadow(bool * pB)
{
   if (pB == NULL)
   {
      return E_POINTER;
   }
   *pB = m_bFontShadow;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontShadow(bool b)
{
   m_bFontShadow = b;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontOutline(bool * pB)
{
   if (pB == NULL)
   {
      return E_POINTER;
   }
   *pB = m_bFontOutline;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetFontOutline(bool b)
{
   m_bFontOutline = b;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontDesc(cGUIFontDesc * pFontDesc)
{
   if (pFontDesc == NULL)
   {
      return E_POINTER;
   }
   if (m_fontName.empty() && m_fontPointSize == 0)
   {
      return S_FALSE;
   }
   uint effects = kGFE_None;
   if (m_bFontBold)
   {
      effects |= kGFE_Bold;
   }
   if (m_bFontItalic)
   {
      effects |= kGFE_Italic;
   }
   if (m_bFontShadow)
   {
      effects |= kGFE_Shadow;
   }
   if (m_bFontOutline)
   {
      effects |= kGFE_Outline;
   }
   *pFontDesc = cGUIFontDesc(m_fontName.c_str(), m_fontPointSize, effects);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetWidth(int * pWidth, uint * pSpec)
{
   if (pWidth == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   if ((m_width == kInvalidUint) || (m_widthSpec == kInvalidUint))
   {
      return S_FALSE;
   }

   *pWidth = m_width;
   *pSpec = m_widthSpec;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetWidth(int width, uint spec)
{
   if ((spec == kGUIDimensionPercent) && (width > 100))
   {
      return E_INVALIDARG;
   }

   m_width = width;
   m_widthSpec = spec;
   
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetHeight(int * pHeight, uint * pSpec)
{
   if (pHeight == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   if ((m_height == kInvalidUint) || (m_heightSpec == kInvalidUint))
   {
      return S_FALSE;
   }

   *pHeight = m_height;
   *pSpec = m_heightSpec;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetHeight(int height, uint spec)
{
   if ((spec == kGUIDimensionPercent) && (height > 100))
   {
      return E_INVALIDARG;
   }

   m_height = height;
   m_heightSpec = spec;

   return S_OK;
}


///////////////////////////////////////

static eGUIAlignment GUIStyleParseAlignment(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, kValueAlignLeft) == 0)
   {
      return kGUIAlignLeft;
   }
   else if (stricmp(psz, kValueAlignRight) == 0)
   {
      return kGUIAlignRight;
   }
   else if (stricmp(psz, kValueAlignCenter) == 0)
   {
      return kGUIAlignCenter;
   }
   else
   {
      return kGUIAlignLeft;
   }
}

static eGUIVerticalAlignment GUIStyleParseVertAlignment(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, kValueVertAlignTop) == 0)
   {
      return kGUIVertAlignTop;
   }
   else if (stricmp(psz, kValueVertAlignBottom) == 0)
   {
      return kGUIVertAlignBottom;
   }
   else if (stricmp(psz, kValueVertAlignCenter) == 0)
   {
      return kGUIVertAlignCenter;
   }
   else
   {
      return kGUIVertAlignTop;
   }
}


///////////////////////////////////////////////////////////////////////////////

static tResult GUIStyleParseAndSetAttribute(const char * pszAttrib, IGUIStyle * pStyle)
{
   Assert(pszAttrib != NULL);
   Assert(!isspace(*pszAttrib));
   Assert(pStyle != NULL);

   const tChar * pszSep = _tcschr(pszAttrib, _T(':'));
   if (pszSep == NULL)
   {
      return E_FAIL;
   }

   const tChar * pszAttribEnd = SkipSpaceBack(pszSep);
   int attribNameLength = pszAttribEnd - pszAttrib;
   tChar * pszAttribName = static_cast<tChar *>(alloca((attribNameLength + 1) * sizeof(tChar)));
   _tcsncpy(pszAttribName, pszAttrib, attribNameLength);
   pszAttribName[attribNameLength] = 0;

   const tChar * pszValueStart = SkipSpaceFwd(pszSep + 1);
   const tChar * pszValueEnd = SkipSpaceBack(pszValueStart + strlen(pszValueStart));
   int valueLength = pszValueEnd - pszValueStart;
   tChar * pszValue = static_cast<tChar *>(alloca((valueLength + 1) * sizeof(tChar)));
   _tcsncpy(pszValue, pszValueStart, valueLength);
   pszValue[valueLength] = 0;

   if (strcmp(pszAttribName, kAttribAlign) == 0)
   {
      return pStyle->SetAlignment(GUIStyleParseAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, kAttribVerticalAlign) == 0)
   {
      return pStyle->SetVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, kAttribBackgroundColor) == 0)
   {
      tGUIColor color;
      if (GUIParseColor(pszValue, &color) == S_OK)
      {
         return pStyle->SetBackgroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, kAttribForegroundColor) == 0)
   {
      tGUIColor color;
      if (GUIParseColor(pszValue, &color) == S_OK)
      {
         return pStyle->SetForegroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, kAttribTextAlign) == 0)
   {
      return pStyle->SetTextAlignment(GUIStyleParseAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, kAttribTextVerticalAlign) == 0)
   {
      return pStyle->SetTextVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, kAttribFontName) == 0)
   {
      return pStyle->SetFontName(pszValue);
   }
   else if (strcmp(pszAttribName, kAttribFontPointSize) == 0)
   {
      int pointSize;
      if (_stscanf(pszValue, _T("%d"), &pointSize) == 1)
      {
         return pStyle->SetFontPointSize(pointSize);
      }
   }
   else if (strcmp(pszAttribName, kAttribFontBold) == 0)
   {
      bool b;
      if (GUIParseBool(pszValue, &b) == S_OK)
      {
         return pStyle->SetFontBold(b);
      }
   }
   else if (strcmp(pszAttribName, kAttribFontItalic) == 0)
   {
      bool b;
      if (GUIParseBool(pszValue, &b) == S_OK)
      {
         return pStyle->SetFontItalic(b);
      }
   }
   else if (strcmp(pszAttribName, kAttribFontShadow) == 0)
   {
      bool b;
      if (GUIParseBool(pszValue, &b) == S_OK)
      {
         return pStyle->SetFontShadow(b);
      }
   }
   else if (strcmp(pszAttribName, kAttribFontOutline) == 0)
   {
      bool b;
      if (GUIParseBool(pszValue, &b) == S_OK)
      {
         return pStyle->SetFontOutline(b);
      }
   }
   else if (strcmp(pszAttribName, kAttribWidth) == 0)
   {
      int width;
      eGUIDimensionSpec spec;
      if (GUIParseStyleDimension(pszValue, &width, &spec) == S_OK)
      {
         return pStyle->SetWidth(width, spec);
      }
   }
   else if (strcmp(pszAttribName, kAttribHeight) == 0)
   {
      int height;
      eGUIDimensionSpec spec;
      if (GUIParseStyleDimension(pszValue, &height, &spec) == S_OK)
      {
         return pStyle->SetHeight(height, spec);
      }
   }
   else
   {
      return pStyle->SetAttribute(pszAttribName, pszValue);
   }

   return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIStyleParse(const tChar * pszStyle, long length, IGUIStyle * * ppStyle)
{
   if (pszStyle == NULL || ppStyle == NULL)
   {
      return E_POINTER;
   }

   if (length < 0)
   {
      length = _tcslen(pszStyle);
   }

   cAutoIPtr<IGUIStyle> pStyle(new cGUIStyle);
   if (!pStyle)
   {
      return E_OUTOFMEMORY;
   }

   const tChar * pszIter = pszStyle;
   const tChar * pszEnd = pszStyle + length;
   while (pszIter < pszEnd)
   {
      pszIter = SkipSpaceFwd(pszIter);

      const tChar * pszIterEnd = _tcspbrk(pszIter, _T(";"));
      if (pszIterEnd == NULL)
      {
         pszIterEnd = pszEnd;
      }

      int attribLength = pszIterEnd - pszIter;
      if (attribLength > 0)
      {
         tChar * pszAttrib = static_cast<tChar *>(alloca((attribLength + 1) * sizeof(tChar)));
         _tcsncpy(pszAttrib, pszIter, attribLength);
         pszAttrib[attribLength] = 0;

         if (FAILED(GUIStyleParseAndSetAttribute(pszAttrib, pStyle)))
         {
            return E_FAIL;
         }
      }

      pszIter = pszIterEnd + 1;
   }

   return pStyle.GetPointer(ppStyle);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cGUIStyleTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cGUIStyleTests);
      CPPUNIT_TEST(TestStyleParse);
      CPPUNIT_TEST(TestCustomAttributes);
      CPPUNIT_TEST(TestParseRejectIdentifiers);
   CPPUNIT_TEST_SUITE_END();

   void TestStyleParse();
   void TestCustomAttributes();
   void TestParseRejectIdentifiers();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIStyleTests);

///////////////////////////////////////

void cGUIStyleTests::TestStyleParse()
{
   uint temp;
   tGUIColor color;
   tGUIString fontName;
   cAutoIPtr<IGUIStyle> pStyle;

   static const tChar szFont[] = _T("MS Sans Serif");
   static const uint kPointSize = 14;

   cStr text;
   text.Format(
      "%s : %s;" \
      "%s : %s;  " \
      "%s : %s;" \
      "%s : (0,0,0) ;" \
      "%s: %s;" \
      "%s: %s;" \
      "%s: %s;" \
      "%s: %d;",
      kAttribAlign, kValueAlignCenter,
      kAttribVerticalAlign, kValueVertAlignCenter,
      kAttribBackgroundColor, kValueColorWhite,
      kAttribForegroundColor,
      kAttribTextAlign, kValueAlignRight,
      kAttribTextVerticalAlign, kValueVertAlignBottom,
      kAttribFontName, szFont,
      kAttribFontPointSize, kPointSize
   );

   CPPUNIT_ASSERT(GUIStyleParse(text.c_str(), text.length(), &pStyle) == S_OK);
   CPPUNIT_ASSERT(pStyle->GetAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIAlignCenter);
   CPPUNIT_ASSERT(pStyle->GetVerticalAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIVertAlignCenter);
   CPPUNIT_ASSERT(pStyle->GetBackgroundColor(&color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::White);
   CPPUNIT_ASSERT(pStyle->GetForegroundColor(&color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor(0,0,0));
   CPPUNIT_ASSERT(pStyle->GetTextAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIAlignRight);
   CPPUNIT_ASSERT(pStyle->GetTextVerticalAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIVertAlignBottom);
   CPPUNIT_ASSERT(pStyle->GetFontName(&fontName) == S_OK);
   CPPUNIT_ASSERT(strcmp(fontName.c_str(), szFont) == 0);
   CPPUNIT_ASSERT(pStyle->GetFontPointSize(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kPointSize);
   SafeRelease(pStyle);
}

///////////////////////////////////////

void cGUIStyleTests::TestCustomAttributes()
{
   tGUIString string;
   uint number;
   tGUIColor color;

   cAutoIPtr<IGUIStyle> pStyle;

   CPPUNIT_ASSERT(GUIStyleParse("", -1, &pStyle) == S_OK);

   // general tests
   CPPUNIT_ASSERT(pStyle->GetAttribute("DOES_NOT_EXIST", &string) == S_FALSE);
   CPPUNIT_ASSERT(pStyle->SetAttribute(NULL, "value") == E_POINTER);
   CPPUNIT_ASSERT(pStyle->SetAttribute("attrib", NULL) == E_POINTER);
   CPPUNIT_ASSERT(pStyle->SetAttribute("", "value") == E_INVALIDARG);

   // test setting a string value
   CPPUNIT_ASSERT(pStyle->SetAttribute("string1", "blah blah blah") == S_OK);
   CPPUNIT_ASSERT(pStyle->GetAttribute("string1", &string) == S_OK);
   CPPUNIT_ASSERT(strcmp(string.c_str(), "blah blah blah") == 0);

   CPPUNIT_ASSERT(pStyle->GetAttribute("string1", &number) == S_FALSE);
   CPPUNIT_ASSERT(pStyle->GetAttribute("string1", &color) == S_FALSE);

   // test changing an existing attribute
   CPPUNIT_ASSERT(pStyle->SetAttribute("string1", "xxx") == S_OK);
   CPPUNIT_ASSERT(pStyle->GetAttribute("string1", &string) == S_OK);
   CPPUNIT_ASSERT(strcmp(string.c_str(), "xxx") == 0);

   // test setting a numeric value
   CPPUNIT_ASSERT(pStyle->SetAttribute("number1", "99") == S_OK);
   CPPUNIT_ASSERT(pStyle->GetAttribute("number1", &string) == S_OK);
   CPPUNIT_ASSERT(strcmp(string.c_str(), "99") == 0);
   CPPUNIT_ASSERT(pStyle->GetAttribute("number1", &number) == S_OK);
   CPPUNIT_ASSERT(number == 99);
   CPPUNIT_ASSERT(pStyle->GetAttribute("number1", &color) == S_FALSE);

   // test setting a color value
   CPPUNIT_ASSERT(pStyle->SetAttribute("color1", "(0.75,0.75,0.75,1)") == S_OK);
   CPPUNIT_ASSERT(pStyle->GetAttribute("color1", &string) == S_OK);
   CPPUNIT_ASSERT(strcmp(string.c_str(), "(0.75,0.75,0.75,1)") == 0);
   CPPUNIT_ASSERT(pStyle->GetAttribute("color1", &number) == S_FALSE);
   CPPUNIT_ASSERT(pStyle->GetAttribute("color1", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor(0.75,0.75,0.75,1));
}

///////////////////////////////////////
// An attribute can specify an inline style as the value of the attribute, or
// refer to a <style> element by identifier. GUIStyleParse should reject
// one-word arguments to help support this.

void cGUIStyleTests::TestParseRejectIdentifiers()
{
   cAutoIPtr<IGUIStyle> pStyle;
   CPPUNIT_ASSERT(GUIStyleParse("myStyle", -1, &pStyle) != S_OK);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
