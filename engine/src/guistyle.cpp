///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"

#include "font.h"
#include "color.h"

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
   m_pFont()
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

   // TODO
   if (stricmp(pszAttribName, "") == 0)
   {
   }

   return S_OK;
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
      "color: Black;" \
      "font-family: MS Gothic, sans-serif;" \
      "font-size: 12px;" \
      "font-weight: normal;" \
      "text-align: left;";

   cAutoIPtr<IGUIStyle> pStyle;

   CPPUNIT_ASSERT(GUIStyleParse(pszTestStyleString, &pStyle) == S_OK);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
