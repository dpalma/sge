///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"

#include "font.h"
#include "color.h"

#include "parse.h"

#include <cstring>
#include <locale>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static const uint NO_DIMENSION = (uint)-1;

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
   m_pFont(NULL),
   m_width(NO_DIMENSION), 
   m_height(NO_DIMENSION),
   m_widthSpec(NO_DIMENSION), 
   m_heightSpec(NO_DIMENSION)
{
}

///////////////////////////////////////

cGUIStyle::~cGUIStyle()
{
   delete m_pBackground, m_pBackground = NULL;
   delete m_pForeground, m_pForeground = NULL;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const char * pszAttribute, tGUIString * pValue)
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

tResult cGUIStyle::GetAttribute(const char * pszAttribute, uint * pValue)
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

   if (sscanf(f->second, "%d", pValue) == 1)
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const char * pszAttribute, tGUIColor * pValue)
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

   if (GUIStyleParseColor(f->second, pValue) == S_OK)
   {
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::SetAttribute(const char * pszAttribute, const char * pszValue)
{
   if (pszAttribute == NULL || pszValue == NULL)
   {
      return E_POINTER;
   }

   if (strlen(pszAttribute) == 0)
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

///////////////////////////////////////

tResult cGUIStyle::GetWidth(uint * pWidth, uint * pSpec)
{
   if (pWidth == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   if ((m_width == NO_DIMENSION) || (m_widthSpec == NO_DIMENSION))
   {
      return S_FALSE;
   }

   *pWidth = m_width;
   *pSpec = m_widthSpec;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetWidth(uint width, uint spec)
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

tResult cGUIStyle::GetHeight(uint * pHeight, uint * pSpec)
{
   if (pHeight == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   if ((m_height == NO_DIMENSION) || (m_heightSpec == NO_DIMENSION))
   {
      return S_FALSE;
   }

   *pHeight = m_height;
   *pSpec = m_heightSpec;

   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetHeight(uint height, uint spec)
{
   if ((spec == kGUIDimensionPercent) && (height > 100))
   {
      return E_INVALIDARG;
   }

   m_height = height;
   m_heightSpec = spec;

   return S_OK;
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

static eGUIAlignment GUIStyleParseAlignment(const char * psz)
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

static eGUIVerticalAlignment GUIStyleParseVertAlignment(const char * psz)
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

tResult GUIStyleParseColor(const char * psz, tGUIColor * pColor)
{
   static const struct sNamedColor
   {
      sNamedColor(const char * p, const tGUIColor & c) : pszName(p), color(c) {}
      const char * pszName;
      tGUIColor color;
   }
   g_namedColorTable[] =
   {
      sNamedColor("black", tGUIColor::Black),
      sNamedColor("red", tGUIColor::Red),
      sNamedColor("green", tGUIColor::Green),
      sNamedColor("yellow", tGUIColor::Yellow),
      sNamedColor("blue", tGUIColor::Blue),
      sNamedColor("magenta", tGUIColor::Magenta),
      sNamedColor("cyan", tGUIColor::Cyan),
      sNamedColor("darkgray", tGUIColor::DarkGray),
      sNamedColor("gray", tGUIColor::Gray),
      sNamedColor("lightgray", tGUIColor::LightGray),
      sNamedColor("white", tGUIColor::White),
   };

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
   else
   {
      for (int i = 0; i < _countof(g_namedColorTable); ++i)
      {
         if (stricmp(g_namedColorTable[i].pszName, psz) == 0)
         {
            *pColor = g_namedColorTable[i].color;
            return S_OK;
         }
      }
   }
   return E_FAIL;
}

///////////////////////////////////////

static tResult GUIStyleParseDimension(const char * psz, uint * pDimension, uint * pSpec)
{
   Assert(psz != NULL);
   Assert(pDimension != NULL);
   Assert(pSpec != NULL);

   if (sscanf(psz, "%d", pDimension) == 1)
   {
      if (strstr(psz, "px"))
      {
         *pSpec = kGUIDimensionPixels;
         return S_OK;
      }
      else if (strchr(psz, '%'))
      {
         *pSpec = kGUIDimensionPercent;
         return S_OK;
      }
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
      return pStyle->SetAlignment(GUIStyleParseAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, "verticalAlign") == 0)
   {
      return pStyle->SetVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, "background-color") == 0)
   {
      tGUIColor color;
      if (GUIStyleParseColor(pszValue, &color) == S_OK)
      {
         return pStyle->SetBackgroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, "foreground-color") == 0)
   {
      tGUIColor color;
      if (GUIStyleParseColor(pszValue, &color) == S_OK)
      {
         return pStyle->SetForegroundColor(color);
      }
   }
   else if (strcmp(pszAttribName, "text-align") == 0)
   {
      return pStyle->SetTextAlignment(GUIStyleParseAlignment(pszValue));
   }
   else if (strcmp(pszAttribName, "text-verticalAlign") == 0)
   {
      return pStyle->SetTextVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
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
   else if (strcmp(pszAttribName, "width") == 0)
   {
      uint width, spec;
      if (GUIStyleParseDimension(pszValue, &width, &spec) == S_OK)
      {
         return pStyle->SetWidth(width, spec);
      }
   }
   else if (strcmp(pszAttribName, "height") == 0)
   {
      uint height, spec;
      if (GUIStyleParseDimension(pszValue, &height, &spec) == S_OK)
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
      CPPUNIT_TEST(TestFactoryFunction);
      CPPUNIT_TEST(TestParseDimension);
      CPPUNIT_TEST(TestCustomAttributes);
      CPPUNIT_TEST(TestParseColor);
   CPPUNIT_TEST_SUITE_END();

   void TestFactoryFunction();
   void TestParseDimension();
   void TestCustomAttributes();
   void TestParseColor();
};

///////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cGUIStyleTests);

///////////////////////////////////////

void cGUIStyleTests::TestFactoryFunction()
{
   uint temp;
   tGUIColor color;
   tGUIString fontName;
   cAutoIPtr<IGUIStyle> pStyle;

   static const char * pszTestStyleString = 
      "align : center;" \
      "verticalAlign : center;  " \
      "background-color : white;" \
      "foreground-color : (0,0,0) ;" \
      "text-align: right;" \
      "text-verticalAlign: bottom;" \
      "font-name: MS Sans Serif;" \
      "font-pointSize: 14;";

   CPPUNIT_ASSERT(GUIStyleParse(pszTestStyleString, &pStyle) == S_OK);
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
   CPPUNIT_ASSERT(strcmp(fontName.c_str(), "MS Sans Serif") == 0);
   CPPUNIT_ASSERT(pStyle->GetFontPointSize(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == 14);
   SafeRelease(pStyle);

   static const char * pszTestStyleString2 = "verticalAlign:bottom;";

   CPPUNIT_ASSERT(GUIStyleParse(pszTestStyleString2, &pStyle) == S_OK);
   CPPUNIT_ASSERT(pStyle->GetVerticalAlignment(&temp) == S_OK);
   CPPUNIT_ASSERT(temp == kGUIVertAlignBottom);
}

///////////////////////////////////////

void cGUIStyleTests::TestParseDimension()
{
   uint dim, spec;
   CPPUNIT_ASSERT(GUIStyleParseDimension("50px", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 50) && (spec == kGUIDimensionPixels));
   CPPUNIT_ASSERT(GUIStyleParseDimension("25%", &dim, &spec) == S_OK);
   CPPUNIT_ASSERT((dim == 25) && (spec == kGUIDimensionPercent));
}

///////////////////////////////////////

void cGUIStyleTests::TestCustomAttributes()
{
   tGUIString string;
   uint number;
   tGUIColor color;

   cAutoIPtr<IGUIStyle> pStyle;

   CPPUNIT_ASSERT(GUIStyleParse("", &pStyle) == S_OK);

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

void cGUIStyleTests::TestParseColor()
{
   tGUIColor color;
   CPPUNIT_ASSERT(GUIStyleParseColor("black", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Black);
   CPPUNIT_ASSERT(GUIStyleParseColor("red", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Red);
   CPPUNIT_ASSERT(GUIStyleParseColor("green", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Green);
   CPPUNIT_ASSERT(GUIStyleParseColor("yellow", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Yellow);
   CPPUNIT_ASSERT(GUIStyleParseColor("blue", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Blue);
   CPPUNIT_ASSERT(GUIStyleParseColor("magenta", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Magenta);
   CPPUNIT_ASSERT(GUIStyleParseColor("cyan", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Cyan);
   CPPUNIT_ASSERT(GUIStyleParseColor("darkgray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::DarkGray);
   CPPUNIT_ASSERT(GUIStyleParseColor("gray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::Gray);
   CPPUNIT_ASSERT(GUIStyleParseColor("lightgray", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::LightGray);
   CPPUNIT_ASSERT(GUIStyleParseColor("white", &color) == S_OK);
   CPPUNIT_ASSERT(color == tGUIColor::White);
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
