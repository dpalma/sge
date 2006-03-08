///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"
#include "guielementtools.h"
#include "guiparse.h"
#include "guistrings.h"

#include "color.h"

#include "globalobj.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cstring>
#include <locale>

#include "dbgalloc.h" // must be last header

static const uint kInvalidUint = ~0u;

static const tChar kStyleAttribNameValueSep = _T(':');

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

///////////////////////////////////////

cGUIStyle::cGUIStyle(IGUIStyle * pClassStyle, IDictionary * pDict)
 : m_pClassStyle(CTAddRef(pClassStyle))
 , m_pDict((pDict != NULL) ? CTAddRef(pDict) : DictionaryCreate(kTransitory))
 , m_alignment(kInvalidUint)
 , m_verticalAlignment(kInvalidUint)
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
 , m_placement(kGUIPlaceRelative)
 , m_width(-1)
 , m_height(-1)
 , m_widthSpec(kInvalidUint)
 , m_heightSpec(kInvalidUint)
{
}

///////////////////////////////////////

cGUIStyle::cGUIStyle(const cGUIStyle & other)
 : m_pClassStyle(other.m_pClassStyle)
 , m_pDict(NULL)
 , m_alignment(other.m_alignment)
 , m_verticalAlignment(other.m_verticalAlignment)
 , m_pBackground(NULL)
 , m_pForeground(NULL)
 , m_textAlignment(other.m_textAlignment)
 , m_textVerticalAlignment(other.m_textVerticalAlignment)
 , m_fontName(other.m_fontName)
 , m_fontPointSize(other.m_fontPointSize)
 , m_bFontBold(other.m_bFontBold)
 , m_bFontItalic(other.m_bFontItalic)
 , m_bFontShadow(other.m_bFontShadow)
 , m_bFontOutline(other.m_bFontOutline)
 , m_placement(other.m_placement)
 , m_width(other.m_width)
 , m_height(other.m_height)
 , m_widthSpec(other.m_widthSpec)
 , m_heightSpec(other.m_heightSpec)
{
   other.m_pDict->Clone(&m_pDict);
   if (other.m_pBackground != NULL)
   {
      m_pBackground = new tGUIColor(*other.m_pBackground);
   }
   if (other.m_pForeground != NULL)
   {
      m_pForeground = new tGUIColor(*other.m_pForeground);
   }
}

///////////////////////////////////////

cGUIStyle::~cGUIStyle()
{
   delete m_pBackground, m_pBackground = NULL;
   delete m_pForeground, m_pForeground = NULL;
}

///////////////////////////////////////

tResult cGUIStyle::Create(IGUIStyle * pClassStyle, IGUIStyle * * ppStyle)
{
   if (ppStyle == NULL)
   {
      return E_POINTER;
   }

   cGUIStyle * pStyle = new cGUIStyle(pClassStyle);
   if (!pStyle || !pStyle->m_pDict)
   {
      return E_OUTOFMEMORY;
   }

   *ppStyle = static_cast<IGUIStyle*>(pStyle);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, tGUIString * pValue)
{
   if (m_pDict->Get(pszAttribute, pValue) == S_OK)
   {
      return S_OK;
   }
   return !!m_pClassStyle ? m_pClassStyle->GetAttribute(pszAttribute, pValue) : S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, int * pValue)
{
   if (m_pDict->Get(pszAttribute, pValue) == S_OK)
   {
      return S_OK;
   }
   return !!m_pClassStyle ? m_pClassStyle->GetAttribute(pszAttribute, pValue) : S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetAttribute(const tChar * pszAttribute, tGUIColor * pValue)
{
   cStr value;
   if (m_pDict->Get(pszAttribute, &value) == S_OK)
   {
      return GUIParseColor(value.c_str(), pValue);
   }
   return !!m_pClassStyle ? m_pClassStyle->GetAttribute(pszAttribute, pValue) : S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::SetAttribute(const tChar * pszAttribute, const tChar * pszValue)
{
   return m_pDict->Set(pszAttribute, pszValue);
}

///////////////////////////////////////

tResult cGUIStyle::SetAttribute(const tChar * pszAttribute, int value)
{
   return m_pDict->Set(pszAttribute, value);
}

///////////////////////////////////////

tResult cGUIStyle::GetAlignment(uint * pAlignment)
{
   if (pAlignment == NULL)
   {
      return E_POINTER;
   }
   if (m_alignment == kInvalidUint)
   {
      return S_FALSE;
   }
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
   if (m_verticalAlignment == kInvalidUint)
   {
      return S_FALSE;
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
      return !!m_pClassStyle ? m_pClassStyle->GetBackgroundColor(pBackground) : S_FALSE;
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
      return !!m_pClassStyle ? m_pClassStyle->GetForegroundColor(pForeground) : S_FALSE;
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
   {
      return E_POINTER;
   }
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
   if (m_fontName.empty())
   {
      return !!m_pClassStyle ? m_pClassStyle->GetFontName(pFontName) : S_FALSE;
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
   {
      return E_POINTER;
   }
   if (m_fontPointSize == 0)
   {
      return !!m_pClassStyle ? m_pClassStyle->GetFontPointSize(pFontPointSize) : S_FALSE;
   }
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

tResult cGUIStyle::GetPlacement(uint * pPlacement) const
{
   if (pPlacement == NULL)
   {
      return E_POINTER;
   }
   if (m_placement == kInvalidUint)
   {
      return S_FALSE;
   }
   *pPlacement = m_placement;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::SetPlacement(uint placement)
{
   if (placement != kGUIPlaceRelative && placement != kGUIPlaceAbsolute)
   {
      return E_INVALIDARG;
   }
   m_placement = placement;
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetWidth(int * pWidth, uint * pSpec)
{
   if (pWidth == NULL || pSpec == NULL)
   {
      return E_POINTER;
   }

   if ((m_width < 0) || (m_widthSpec == kInvalidUint))
   {
      return !!m_pClassStyle ? m_pClassStyle->GetWidth(pWidth, pSpec) : S_FALSE;
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

   if ((m_height < 0) || (m_heightSpec == kInvalidUint))
   {
      return !!m_pClassStyle ? m_pClassStyle->GetHeight(pHeight, pSpec) : S_FALSE;
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

tResult cGUIStyle::Clone(IGUIStyle * * ppStyle)
{
   if (ppStyle == NULL)
   {
      return E_POINTER;
   }

   cGUIStyle * pStyle = new cGUIStyle(*this);
   if (pStyle == NULL || !pStyle->m_pDict)
   {
      return E_OUTOFMEMORY;
   }

   *ppStyle = static_cast<IGUIStyle *>(pStyle);
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

///////////////////////////////////////

static tResult GUIStyleParseAndSetAlignment(const char * pszValue, IGUIStyle * pStyle)
{
   return pStyle->SetAlignment(GUIStyleParseAlignment(pszValue));
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetVertAlignment(const char * pszValue, IGUIStyle * pStyle)
{
   return pStyle->SetVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetBackgroundColor(const char * pszValue, IGUIStyle * pStyle)
{
   tGUIColor color;
   if (GUIParseColor(pszValue, &color) == S_OK)
   {
      return pStyle->SetBackgroundColor(color);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetForegroundColor(const char * pszValue, IGUIStyle * pStyle)
{
   tGUIColor color;
   if (GUIParseColor(pszValue, &color) == S_OK)
   {
      return pStyle->SetForegroundColor(color);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetTextAlign(const char * pszValue, IGUIStyle * pStyle)
{
   return pStyle->SetTextAlignment(GUIStyleParseAlignment(pszValue));
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetTextVerticalAlign(const char * pszValue, IGUIStyle * pStyle)
{
   return pStyle->SetTextVerticalAlignment(GUIStyleParseVertAlignment(pszValue));
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontName(const char * pszValue, IGUIStyle * pStyle)
{
   return pStyle->SetFontName(pszValue);
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontPointSize(const char * pszValue, IGUIStyle * pStyle)
{
   int pointSize;
   if (_stscanf(pszValue, _T("%d"), &pointSize) == 1)
   {
      return pStyle->SetFontPointSize(pointSize);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontBold(const char * pszValue, IGUIStyle * pStyle)
{
   bool b;
   if (GUIParseBool(pszValue, &b) == S_OK)
   {
      return pStyle->SetFontBold(b);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontItalic(const char * pszValue, IGUIStyle * pStyle)
{
   bool b;
   if (GUIParseBool(pszValue, &b) == S_OK)
   {
      return pStyle->SetFontItalic(b);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontShadow(const char * pszValue, IGUIStyle * pStyle)
{
   bool b;
   if (GUIParseBool(pszValue, &b) == S_OK)
   {
      return pStyle->SetFontShadow(b);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetFontOutline(const char * pszValue, IGUIStyle * pStyle)
{
   bool b;
   if (GUIParseBool(pszValue, &b) == S_OK)
   {
      return pStyle->SetFontOutline(b);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetPlacement(const char * pszValue, IGUIStyle * pStyle)
{
   if (stricmp(pszValue, kValueRelative) == 0)
   {
      return pStyle->SetPlacement(kGUIPlaceRelative);
   }
   else if (stricmp(pszValue, kValueAbsolute) == 0)
   {
      return pStyle->SetPlacement(kGUIPlaceAbsolute);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetWidth(const char * pszValue, IGUIStyle * pStyle)
{
   int width;
   eGUIDimensionSpec spec;
   if (GUIParseStyleDimension(pszValue, &width, &spec) == S_OK)
   {
      return pStyle->SetWidth(width, spec);
   }
   return S_FALSE;
}

///////////////////////////////////////

static tResult GUIStyleParseAndSetHeight(const char * pszValue, IGUIStyle * pStyle)
{
   int height;
   eGUIDimensionSpec spec;
   if (GUIParseStyleDimension(pszValue, &height, &spec) == S_OK)
   {
      return pStyle->SetHeight(height, spec);
   }
   return S_FALSE;
}

///////////////////////////////////////

typedef tResult (* tGUIStyleParseAndSetAttribFn)(const char * pszValue, IGUIStyle * pStyle);

static const struct
{
   const tChar * pszAttrib;
   tGUIStyleParseAndSetAttribFn pfnParseAndSet;
}
g_GUIStyleParseAndSetAttribTable[] =
{
   { kAttribAlign,               GUIStyleParseAndSetAlignment },
   { kAttribVerticalAlign,       GUIStyleParseAndSetVertAlignment },
   { kAttribBackgroundColor,     GUIStyleParseAndSetBackgroundColor },
   { kAttribForegroundColor,     GUIStyleParseAndSetForegroundColor },
   { kAttribTextAlign,           GUIStyleParseAndSetTextAlign },
   { kAttribTextVerticalAlign,   GUIStyleParseAndSetTextVerticalAlign },
   { kAttribFontName,            GUIStyleParseAndSetFontName },
   { kAttribFontPointSize,       GUIStyleParseAndSetFontPointSize },
   { kAttribFontBold,            GUIStyleParseAndSetFontBold },
   { kAttribFontItalic,          GUIStyleParseAndSetFontItalic },
   { kAttribFontShadow,          GUIStyleParseAndSetFontShadow },
   { kAttribFontOutline,         GUIStyleParseAndSetFontOutline },
   { kAttribPlacement,           GUIStyleParseAndSetPlacement },
   { kAttribWidth,               GUIStyleParseAndSetWidth },
   { kAttribHeight,              GUIStyleParseAndSetHeight },
};

///////////////////////////////////////

static tResult GUIStyleParseAndSetAttribute(const char * pszAttrib, IGUIStyle * pStyle)
{
   Assert(pszAttrib != NULL);
   Assert(!isspace(*pszAttrib));
   Assert(pStyle != NULL);

   const tChar * pszSep = _tcschr(pszAttrib, kStyleAttribNameValueSep);
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

   for (int i = 0; i < _countof(g_GUIStyleParseAndSetAttribTable); i++)
   {
      if (strcmp(pszAttribName, g_GUIStyleParseAndSetAttribTable[i].pszAttrib) == 0)
      {
         tResult result = (*g_GUIStyleParseAndSetAttribTable[i].pfnParseAndSet)(pszValue, pStyle);
         WarnMsgIf2(result != S_OK, "Invalid value \"%s\" for attribute \"%s\"\n", pszValue, pszAttribName);
         return result;
      }
   }

   return pStyle->SetAttribute(pszAttribName, pszValue);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIStyleParse(const tChar * pszStyle, long length, IGUIStyle * * ppStyle)
{
   return GUIStyleParseInline(pszStyle, length, NULL, ppStyle);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIStyleParseInline(const tChar * pszStyle, long length, IGUIStyle * pClassStyle, IGUIStyle * * ppStyle)
{
   if (pszStyle == NULL || ppStyle == NULL)
   {
      return E_POINTER;
   }

   if (length < 0)
   {
      length = _tcslen(pszStyle);
   }

   cAutoIPtr<IGUIStyle> pStyle;
   tResult result = cGUIStyle::Create(pClassStyle, &pStyle);
   if (result != S_OK)
   {
      return result;
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

#ifdef HAVE_CPPUNITLITE2

class cGUIStyleTests
{
public:
   cGUIStyleTests();
   ~cGUIStyleTests();

   bool StyleMatchesTest(IGUIStyle * pStyle);

   cStr m_testStyle;

   static const tChar gm_fontName[];
   static const uint gm_fontPointSize;
};

///////////////////////////////////////

const tChar cGUIStyleTests::gm_fontName[] = _T("MS Sans Serif");
const uint cGUIStyleTests::gm_fontPointSize = 14;

///////////////////////////////////////

cGUIStyleTests::cGUIStyleTests()
{
   Sprintf(&m_testStyle, _T(
      "%s : %s;" \
      "%s : %s;  " \
      "%s : %s;" \
      "%s : (0,0,0) ;" \
      "%s: %s;" \
      "%s: %s;" \
      "%s: %s;" \
      "%s: %d;"),
      kAttribAlign, kValueAlignCenter,
      kAttribVerticalAlign, kValueVertAlignCenter,
      kAttribBackgroundColor, kValueColorWhite,
      kAttribForegroundColor,
      kAttribTextAlign, kValueAlignRight,
      kAttribTextVerticalAlign, kValueVertAlignBottom,
      kAttribFontName, gm_fontName,
      kAttribFontPointSize, gm_fontPointSize
   );
}

///////////////////////////////////////

cGUIStyleTests::~cGUIStyleTests()
{
}

///////////////////////////////////////

TEST_F(cGUIStyleTests, TestClone)
{
   cAutoIPtr<IGUIStyle> pStyle;
   CHECK(GUIStyleParse(m_testStyle.c_str(), m_testStyle.length(), &pStyle) == S_OK);
   CHECK(StyleMatchesTest(pStyle));
   CHECK(pStyle->SetAttribute("ninety-nine", 99) == S_OK);
//   CHECK(pStyle->SetAttribute("pi", kPi) == S_OK);

   cAutoIPtr<IGUIStyle> pClone;
   CHECK(pStyle->Clone(&pClone) == S_OK);
   CHECK(StyleMatchesTest(pClone));

   int i;
   CHECK(pClone->GetAttribute("ninety-nine", &i) == S_OK && i == 99);

//   float f;
//   CHECK(pStyle->GetAttribute("pi", &f) == S_OK && f == kPi);
}

///////////////////////////////////////

TEST_F(cGUIStyleTests, TestStyleParse)
{
   cAutoIPtr<IGUIStyle> pStyle;
   CHECK(GUIStyleParse(m_testStyle.c_str(), m_testStyle.length(), &pStyle) == S_OK);
   CHECK(StyleMatchesTest(pStyle));
   SafeRelease(pStyle);
}

///////////////////////////////////////

TEST_F(cGUIStyleTests, TestCustomAttributes)
{
   tGUIString string;
   int number;
   tGUIColor color;

   cAutoIPtr<IGUIStyle> pStyle;

   CHECK(GUIStyleParse("", -1, &pStyle) == S_OK);

   // general tests
   CHECK(pStyle->GetAttribute("DOES_NOT_EXIST", &string) == S_FALSE);
   CHECK(pStyle->SetAttribute(NULL, "value") == E_POINTER);
   CHECK(pStyle->SetAttribute("attrib", static_cast<const tChar *>(NULL)) == E_POINTER);
   CHECK(pStyle->SetAttribute("", "value") == E_INVALIDARG);

   // test setting a string value
   CHECK(pStyle->SetAttribute("string1", "blah blah blah") == S_OK);
   CHECK(pStyle->GetAttribute("string1", &string) == S_OK);
   CHECK(strcmp(string.c_str(), "blah blah blah") == 0);

   CHECK(pStyle->GetAttribute("string1", &number) == S_FALSE);
   CHECK(pStyle->GetAttribute("string1", &color) == S_FALSE);

   // test changing an existing attribute
   CHECK(pStyle->SetAttribute("string1", "xxx") == S_OK);
   CHECK(pStyle->GetAttribute("string1", &string) == S_OK);
   CHECK(strcmp(string.c_str(), "xxx") == 0);

   // test setting a numeric value
   CHECK(pStyle->SetAttribute("number1", "99") == S_OK);
   CHECK(pStyle->GetAttribute("number1", &string) == S_OK);
   CHECK(strcmp(string.c_str(), "99") == 0);
   CHECK(pStyle->GetAttribute("number1", &number) == S_OK);
   CHECK_EQUAL(number, 99);
   CHECK(pStyle->GetAttribute("number1", &color) == S_FALSE);

   // test setting a color value
   CHECK(pStyle->SetAttribute("color1", "(0.75,0.75,0.75,1)") == S_OK);
   CHECK(pStyle->GetAttribute("color1", &string) == S_OK);
   CHECK(strcmp(string.c_str(), "(0.75,0.75,0.75,1)") == 0);
   CHECK(pStyle->GetAttribute("color1", &number) == S_FALSE);
   CHECK(pStyle->GetAttribute("color1", &color) == S_OK);
   CHECK(color == tGUIColor(0.75,0.75,0.75,1));
}

///////////////////////////////////////
// An attribute can specify an inline style as the value of the attribute, or
// refer to a <style> element by identifier. GUIStyleParse should reject
// one-word arguments to help support this.

TEST(GUIStyleParseRejectIdentifiers)
{
   cAutoIPtr<IGUIStyle> pStyle;
   CHECK(GUIStyleParse("myStyle", -1, &pStyle) != S_OK);
}

///////////////////////////////////////

bool cGUIStyleTests::StyleMatchesTest(IGUIStyle * pStyle)
{
   if (pStyle != NULL)
   {
      uint temp;
      tGUIColor color;
      tGUIString str;

      if ((pStyle->GetAlignment(&temp) == S_OK) && (temp == kGUIAlignCenter)
         && (pStyle->GetVerticalAlignment(&temp) == S_OK) && (temp == kGUIVertAlignCenter)
         && (pStyle->GetBackgroundColor(&color) == S_OK) && (color == GUIStandardColors::White)
         && (pStyle->GetForegroundColor(&color) == S_OK) && (color == tGUIColor(0,0,0))
         && (pStyle->GetTextAlignment(&temp) == S_OK) && (temp == kGUIAlignRight)
         && (pStyle->GetTextVerticalAlignment(&temp) == S_OK) && (temp == kGUIVertAlignBottom)
         && (pStyle->GetFontName(&str) == S_OK) && (strcmp(str.c_str(), gm_fontName) == 0)
         && (pStyle->GetFontPointSize(&temp) == S_OK) && (temp == gm_fontPointSize))
      {
         return true;
      }
   }
   return false;
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
