///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"

#include "font.h"
#include "color.h"

#include "parse.h"

#include <cstring>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

///////////////////////////////////////

cGUIStyle::cGUIStyle()
 : m_alignment(kGUIAlignLeft), 
   m_verticalAlignment(kGUIVertAlignTop),
   m_pBackground(NULL), 
   m_pForeground(NULL),
   m_textAlignment(kGUIAlignLeft), 
   m_textVerticalAlignment(kGUIVertAlignTop),
   m_fontName(""),
   m_fontPointSize(0),
   m_pFont(NULL)
{
}

///////////////////////////////////////

cGUIStyle::~cGUIStyle()
{
   delete m_pBackground, m_pBackground = NULL;
   delete m_pForeground, m_pForeground = NULL;
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
      return E_POINTER;
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
   SafeRelease(m_pFont);
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
   SafeRelease(m_pFont);
   m_fontPointSize = fontPointSize;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFont(IRenderFont * * ppFont)
{
   if (!m_pFont)
   {
      if ((m_fontName.length() == 0) || (m_fontPointSize == 0))
      {
         return E_FAIL;
      }
      m_pFont = FontCreate(m_fontName.c_str(), m_fontPointSize);
   }
   return m_pFont.GetPointer(ppFont);
}


///////////////////////////////////////////////////////////////////////////////

inline const char * SkipSpaceFwd(const char * psz)
{
   Assert(psz != NULL);
   while (isspace(*psz))
      psz++;
   return psz;
}

inline const char * SkipSpaceBack(const char * psz)
{
   Assert(psz != NULL);
   while (isspace(*(psz - 1)))
      psz--;
   return psz;
}

///////////////////////////////////////

static eGUIAlignment String2Align(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, "left") == 0)
   {
      return kGUIAlignLeft;
   }
   else if (stricmp(psz, "right") == 0)
   {
      return kGUIAlignRight;
   }
   else if (stricmp(psz, "center") == 0)
   {
      return kGUIAlignCenter;
   }
   else
   {
      return kGUIAlignLeft;
   }
}

static eGUIVerticalAlignment String2VAlign(const char * psz)
{
   Assert(psz != NULL);
   if (stricmp(psz, "top") == 0)
   {
      return kGUIVertAlignTop;
   }
   else if (stricmp(psz, "bottom") == 0)
   {
      return kGUIVertAlignBottom;
   }
   else if (stricmp(psz, "center") == 0)
   {
      return kGUIVertAlignCenter;
   }
   else
   {
      return kGUIVertAlignTop;
   }
}

///////////////////////////////////////

static tResult String2Color(const char * psz, tGUIColor * pColor)
{
   double rgba[4];
   int parseResult = ParseTuple(psz, rgba, _countof(rgba));
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
   else if (stricmp(psz, "black") == 0)
   {
      *pColor = tGUIColor::Black;
      return S_OK;
   }
   else if (stricmp(psz, "red") == 0)
   {
      *pColor = tGUIColor::Red;
      return S_OK;
   }
   else if (stricmp(psz, "green") == 0)
   {
      *pColor = tGUIColor::Green;
      return S_OK;
   }
   else if (stricmp(psz, "yellow") == 0)
   {
      *pColor = tGUIColor::Yellow;
      return S_OK;
   }
   else if (stricmp(psz, "blue") == 0)
   {
      *pColor = tGUIColor::Blue;
      return S_OK;
   }
   else if (stricmp(psz, "magenta") == 0)
   {
      *pColor = tGUIColor::Magenta;
      return S_OK;
   }
   else if (stricmp(psz, "cyan") == 0)
   {
      *pColor = tGUIColor::Cyan;
      return S_OK;
   }
   else if (stricmp(psz, "white") == 0)
   {
      *pColor = tGUIColor::White;
      return S_OK;
   }
   return E_FAIL;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetAttribute(const char * pszAttrib, IGUIStyle * pStyle)
{
   Assert(pszAttrib != NULL);
   Assert(!isspace(*pszAttrib));
   Assert(pStyle != NULL);

   const char * pszSep = strchr(pszAttrib, ':');
   if (pszSep == NULL)
   {
      return E_FAIL;
   }

   const char * pszAttribEnd = SkipSpaceBack(pszSep);
   int attribNameLength = pszAttribEnd - pszAttrib;
   char * pszAttribName = static_cast<char *>(alloca(attribNameLength + 1));
   strncpy(pszAttribName, pszAttrib, attribNameLength);
   pszAttribName[attribNameLength] = 0;

   const char * pszValueStart = SkipSpaceFwd(pszSep + 1);
   const char * pszValueEnd = SkipSpaceBack(pszValueStart + strlen(pszValueStart));
   int valueLength = pszValueEnd - pszValueStart;
   char * pszValue = static_cast<char *>(alloca(valueLength + 1));
   strncpy(pszValue, pszValueStart, valueLength);
   pszValue[valueLength] = 0;

   if (strcmp(pszAttribName, "align") == 0)
   {
      return pStyle->SetAlignment(String2Align(pszValue));
   }
   else if (strcmp(pszAttribName, "verticalAlign") == 0)
   {
      return pStyle->SetVerticalAlignment(String2VAlign(pszValue));
   }
   else if (strcmp(pszAttribName, "background-color") == 0)
   {
      tGUIColor color;
      if (String2Color(pszValue, &color) == S_OK)
      {
         return pStyle->SetBackgroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, "foreground-color") == 0)
   {
      tGUIColor color;
      if (String2Color(pszValue, &color) == S_OK)
      {
         return pStyle->SetForegroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, "text-align") == 0)
   {
      return pStyle->SetTextAlignment(String2Align(pszValue));
   }
   else if (strcmp(pszAttribName, "text-verticalAlign") == 0)
   {
      return pStyle->SetTextVerticalAlignment(String2VAlign(pszValue));
   }
   else if (strcmp(pszAttribName, "font-name") == 0)
   {
      return pStyle->SetFontName(pszValue);
   }
   else if (strcmp(pszAttribName, "font-pointSize") == 0)
   {
      int pointSize;
      if (sscanf(pszValue, "%d", &pointSize) == 1)
      {
         return pStyle->SetFontPointSize(pointSize);
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult GUIStyleParse(const char * pszStyle, IGUIStyle * * ppStyle)
{
   if (pszStyle == NULL || ppStyle == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyle> pStyle(new cGUIStyle);

   if (!pStyle)
   {
      return E_OUTOFMEMORY;
   }

   const char * pszIter = pszStyle;
   const char * pszEnd = pszStyle + strlen(pszStyle);
   while (pszIter < pszEnd)
   {
      pszIter = SkipSpaceFwd(pszIter);

      const char * pszIterEnd = strpbrk(pszIter, ";");
      if (pszIterEnd == NULL)
         pszIterEnd = pszEnd;

      int attribLength = pszIterEnd - pszIter;

      if (attribLength > 0)
      {
         char * pszAttrib = static_cast<char *>(alloca(attribLength + 1));
         strncpy(pszAttrib, pszIter, attribLength);
         pszAttrib[attribLength] = 0;

         if (FAILED(GUIStyleParseAndSetAttribute(pszAttrib, pStyle)))
         {
            DebugMsg1("WARNING: Error in parsing attribute \"%s\"\n", pszAttrib);
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
      CPPUNIT_TEST(Test);
   CPPUNIT_TEST_SUITE_END();

   void Test();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIStyleTests);

///////////////////////////////////////

void cGUIStyleTests::Test()
{
   const char * pszTestStyleString = 
      "align : center;" \
      "verticalAlign : center;  " \
      "background-color : white;" \
      "foreground-color : (0,0,0) ;" \
      "text-align: right;" \
      "text-verticalAlign: bottom;" \
      "font-name: MS Sans Serif;" \
      "font-pointSize: 14;";

   cAutoIPtr<IGUIStyle> pStyle;

   CPPUNIT_ASSERT(GUIStyleParse(pszTestStyleString, &pStyle) == S_OK);
   uint temp;
   CPPUNIT_ASSERT(pStyle->GetAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIAlignCenter);
   CPPUNIT_ASSERT(pStyle->GetVerticalAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIVertAlignCenter);
   tGUIColor color;
   CPPUNIT_ASSERT(pStyle->GetBackgroundColor(&color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::White);
   CPPUNIT_ASSERT(pStyle->GetForegroundColor(&color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor(0,0,0));
   CPPUNIT_ASSERT(pStyle->GetTextAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIAlignRight);
   CPPUNIT_ASSERT(pStyle->GetTextVerticalAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIVertAlignBottom);
   tGUIString fontName;
   CPPUNIT_ASSERT(pStyle->GetFontName(&fontName) == S_OK);
   CPPUNIT_ASSERT(strcmp(fontName.c_str(), "MS Sans Serif") == 0);
   CPPUNIT_ASSERT(pStyle->GetFontPointSize(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == 14);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
