///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guistyle.h"
#include "gui/guiapi.h"
#include "guielementtools.h"
#include "guiparse.h"
#include "guistrings.h"

#include "render/renderapi.h"
#include "render/renderfontapi.h"

#include "tech/globalobj.h"
#include "tech/token.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstring>
#include <locale>

#include "tech/dbgalloc.h" // must be last header

static const uint kInvalidUint = ~0u;

static const tChar kStyleAttribNameValueSep = _T(':');


///////////////////////////////////////////////////////////////////////////////

static tResult GUIStyleFontDesc(IGUIStyle * pStyle, cStr * pFontName, int * pPointSize, uint * pFlags)
{
   if (pStyle == NULL || pFontName == NULL || pPointSize == NULL || pFlags == NULL)
   {
      return E_POINTER;
   }

   if (pStyle->GetFontName(pFontName) != S_OK)
   {
      return E_FAIL;
   }

   int size;
   uint sizeType;
   if (pStyle->GetFontSize(&size, &sizeType) != S_OK)
   {
      return E_FAIL;
   }

   if (sizeType == kGUIFontSizePoints)
   {
      *pPointSize = size;
   }
   else
   {
      // TODO: convert to points
      return E_INVALIDARG;
   }

   uint flags = kRFF_None;

   {
      bool bBold = false;
      if ((pStyle->GetFontBold(&bBold) == S_OK) && bBold)
      {
         flags |= kRFF_Bold;
      }
   }

   {
      bool bItalic = false;
      if ((pStyle->GetFontItalic(&bItalic) == S_OK) && bItalic)
      {
         flags |= kRFF_Italic;
      }
   }

   {
      bool bShadow = false;
      if ((pStyle->GetFontShadow(&bShadow) == S_OK) && bShadow)
      {
         flags |= kRFF_Shadow;
      }
   }

   {
      bool bOutline = false;
      if ((pStyle->GetFontOutline(&bOutline) == S_OK) && bOutline)
      {
         flags |= kRFF_Outline;
      }
   }

   *pFlags = flags;

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIStyle
//

///////////////////////////////////////

cGUIStyle::cGUIStyle(IGUIStyle * pClassStyle, IDictionary * pDict)
 : m_pClassStyle(CTAddRef(pClassStyle))
 , m_pDict(CTAddRef(pDict))
 , m_alignment(kInvalidUint)
 , m_verticalAlignment(kInvalidUint)
 , m_pBackground(NULL)
 , m_pForeground(NULL)
 , m_textAlignment(kGUIAlignLeft)
 , m_textVerticalAlignment(kGUIVertAlignTop)
 , m_fontName("")
 , m_fontSize(0)
 , m_fontSizeType(0)
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
   if (!m_pDict)
   {
      DictionaryCreate(kTransitory, &m_pDict);
   }
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
 , m_fontSize(other.m_fontSize)
 , m_fontSizeType(other.m_fontSizeType)
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
   if (pszAttribute == NULL)
   {
      return E_POINTER;
   }

   tColorMap::const_iterator f = m_colorMap.find(pszAttribute);
   if (f != m_colorMap.end())
   {
      if (pValue != NULL)
      {
         *pValue = f->second;
      }
      return S_OK;
   }

   cStr value;
   if (m_pDict->Get(pszAttribute, &value) == S_OK)
   {
      tGUIColor color;
      tResult result = GUIParseColor(value.c_str(), &color);
      if (result == S_OK)
      {
         m_colorMap[pszAttribute] = color;
         if (pValue != NULL)
         {
            *pValue = color;
         }
      }
      return result;
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
   if (m_pBackground == NULL)
   {
      m_pBackground = new tGUIColor(background);
      if (m_pBackground == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return S_OK;
   }
   else
   {
      *m_pBackground = background;
      return S_OK;
   }
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
   if (m_pForeground == NULL)
   {
      m_pForeground = new tGUIColor(foreground);
      if (m_pForeground == NULL)
      {
         return E_OUTOFMEMORY;
      }
      return S_OK;
   }
   else
   {
      *m_pForeground = foreground;
      return S_OK;
   }
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
   SafeRelease(m_pCachedFont);
   return m_fontName.length() > 0 ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cGUIStyle::GetFontSize(int * pSize, uint * pSizeType)
{
   if (pSize == NULL || pSizeType == NULL)
   {
      return E_POINTER;
   }
   if (m_fontSize != 0 && m_fontSizeType != kGUIFontSizeTypeUnspecified)
   {
      *pSize = m_fontSize;
      *pSizeType = m_fontSizeType;
      return S_OK;
   }
   return !!m_pClassStyle ? m_pClassStyle->GetFontSize(pSize, pSizeType) : S_FALSE;
}

///////////////////////////////////////

static bool ValidateFontSize(int size, uint sizeType)
{
   if (sizeType == kGUIFontSizePoints)
   {
      return ((size >= 6) && (size <= 48));
   }
   // TODO: validate other font size types
   return true;
}

tResult cGUIStyle::SetFontSize(int size, uint sizeType)
{
   if (!ValidateFontSize(size, sizeType))
   {
      ErrorMsg2("Odd font size requested: %d, %d\n", size, sizeType);
      return E_INVALIDARG;
   }
   m_fontSize = size;
   m_fontSizeType = sizeType;
   SafeRelease(m_pCachedFont);
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
   SafeRelease(m_pCachedFont);
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
   SafeRelease(m_pCachedFont);
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
   SafeRelease(m_pCachedFont);
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
   SafeRelease(m_pCachedFont);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIStyle::GetFont(IRenderFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   if (!!m_pCachedFont)
   {
      return m_pCachedFont.GetPointer(ppFont);
   }

   tGUIString fontFamily;
   if (GetFontName(&fontFamily) != S_OK)
   {
      return E_FAIL;
   }

   cTokenizer<cStr> tok;
   if (tok.Tokenize(fontFamily.c_str(), _T(",")) <= 0)
   {
      return E_FAIL;
   }

   cStr fontName;
   int pointSize = 0;
   uint flags = kRFF_None;
   if (GUIStyleFontDesc(static_cast<IGUIStyle*>(this), &fontName, &pointSize, &flags) != S_OK)
   {
      return E_FAIL;
   }

   UseGlobal(Renderer);

   Assert(!m_pCachedFont);

   for (uint i = 0; i < tok.m_tokens.size(); i++)
   {
      fontName = tok.m_tokens[i];
      if (pRenderer->CreateFont(fontName.c_str(), pointSize, flags, &m_pCachedFont) == S_OK)
      {
         return m_pCachedFont.GetPointer(ppFont);
      }
   }

   return E_FAIL;
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
   if (_stricmp(psz, kValueAlignLeft) == 0)
   {
      return kGUIAlignLeft;
   }
   else if (_stricmp(psz, kValueAlignRight) == 0)
   {
      return kGUIAlignRight;
   }
   else if (_stricmp(psz, kValueAlignCenter) == 0)
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
   if (_stricmp(psz, kValueVertAlignTop) == 0)
   {
      return kGUIVertAlignTop;
   }
   else if (_stricmp(psz, kValueVertAlignBottom) == 0)
   {
      return kGUIVertAlignBottom;
   }
   else if (_stricmp(psz, kValueVertAlignCenter) == 0)
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

static tResult GUIStyleParseAndSetFontSize(const char * pszValue, IGUIStyle * pStyle)
{
   int size = 0;
   eGUIFontSizeType type = kGUIFontSizeTypeUnspecified;
   if (GUIParseStyleFontSize(pszValue, &size, &type) == S_OK)
   {
      return pStyle->SetFontSize(size, type);
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
   if (_stricmp(pszValue, kValueRelative) == 0)
   {
      return pStyle->SetPlacement(kGUIPlaceRelative);
   }
   else if (_stricmp(pszValue, kValueAbsolute) == 0)
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
   { kAttribFontSize,            GUIStyleParseAndSetFontSize },
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
   cStr attribName(pszAttrib, attribNameLength);

   const tChar * pszValueStart = SkipSpaceFwd(pszSep + 1);
   const tChar * pszValueEnd = SkipSpaceBack(pszValueStart + strlen(pszValueStart));
   int valueLength = pszValueEnd - pszValueStart;
   cStr attribValue(pszValueStart, valueLength);

   for (int i = 0; i < _countof(g_GUIStyleParseAndSetAttribTable); i++)
   {
      if (strcmp(attribName.c_str(), g_GUIStyleParseAndSetAttribTable[i].pszAttrib) == 0)
      {
         tResult result = (*g_GUIStyleParseAndSetAttribTable[i].pfnParseAndSet)(attribValue.c_str(), pStyle);
         WarnMsgIf2(result != S_OK, "Invalid value \"%s\" for attribute \"%s\"\n", attribValue.c_str(), attribName.c_str());
         return result;
      }
   }

   return pStyle->SetAttribute(attribName.c_str(), attribValue.c_str());
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
         //tChar * pszAttrib = static_cast<tChar *>(alloca((attribLength + 1) * sizeof(tChar)));
         //_tcsncpy(pszAttrib, pszIter, attribLength);
         //pszAttrib[attribLength] = 0;
         cStr attrib(pszIter, attribLength);

         if (FAILED(GUIStyleParseAndSetAttribute(attrib.c_str(), pStyle)))
         {
            return E_FAIL;
         }
      }

      pszIter = pszIterEnd + 1;
   }

   return pStyle.GetPointer(ppStyle);
}


///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

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
      "%s: %dpt;"),
      kAttribAlign, kValueAlignCenter,
      kAttribVerticalAlign, kValueVertAlignCenter,
      kAttribBackgroundColor, kValueColorWhite,
      kAttribForegroundColor,
      kAttribTextAlign, kValueAlignRight,
      kAttribTextVerticalAlign, kValueVertAlignBottom,
      kAttribFontName, gm_fontName,
      kAttribFontSize, gm_fontPointSize
   );
}

///////////////////////////////////////

cGUIStyleTests::~cGUIStyleTests()
{
}

///////////////////////////////////////

TEST_FIXTURE(cGUIStyleTests, TestClone)
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

TEST_FIXTURE(cGUIStyleTests, TestStyleParse)
{
   cAutoIPtr<IGUIStyle> pStyle;
   CHECK(GUIStyleParse(m_testStyle.c_str(), m_testStyle.length(), &pStyle) == S_OK);
   CHECK(StyleMatchesTest(pStyle));
   SafeRelease(pStyle);
}

///////////////////////////////////////

TEST_FIXTURE(cGUIStyleTests, TestCustomAttributes)
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
      int fontSize;
      uint fontSizeType;

      if ((pStyle->GetAlignment(&temp) == S_OK) && (temp == kGUIAlignCenter)
         && (pStyle->GetVerticalAlignment(&temp) == S_OK) && (temp == kGUIVertAlignCenter)
         && (pStyle->GetBackgroundColor(&color) == S_OK) && (color == GUIStandardColors::White)
         && (pStyle->GetForegroundColor(&color) == S_OK) && (color == tGUIColor(0,0,0))
         && (pStyle->GetTextAlignment(&temp) == S_OK) && (temp == kGUIAlignRight)
         && (pStyle->GetTextVerticalAlignment(&temp) == S_OK) && (temp == kGUIVertAlignBottom)
         && (pStyle->GetFontName(&str) == S_OK) && (strcmp(str.c_str(), gm_fontName) == 0)
         && (pStyle->GetFontSize(&fontSize, &fontSizeType) == S_OK) && (fontSize == gm_fontPointSize) && (fontSizeType == kGUIFontSizePoints))
      {
         return true;
      }
   }
   return false;
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
