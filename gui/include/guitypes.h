///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITYPES_H
#define INCLUDED_GUITYPES_H

#include "guidll.h"

#include "color.h"
#include "rect.h"
#include "techstring.h"
#include "vec2.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef cStr tGUIString;
typedef tChar tGUIChar;

typedef tVec2 tGUIPoint;

typedef cVec2<int> tScreenPoint;

template <typename T> class cGUISize;
typedef float tGUISizeType;
typedef class cGUISize<tGUISizeType> tGUISize;

typedef tRect tGUIRect;

typedef cColor tGUIColor;


///////////////////////////////////////////////////////////////////////////////

enum eGUIPlacement
{
   kGUIPlaceRelative,
   kGUIPlaceAbsolute,
};

enum eGUIAlignment
{
   kGUIAlignLeft,
   kGUIAlignRight,
   kGUIAlignCenter,
};

enum eGUIVerticalAlignment
{
   kGUIVertAlignTop,
   kGUIVertAlignBottom,
   kGUIVertAlignCenter,
};

enum eGUIDimensionSpec
{
   kGUIDimensionPixels,
   kGUIDimensionPercent,
};

enum eGUIFontSizeType
{
   kGUIFontSizeTypeUnspecified,
   kGUIFontSizeAbsolute,
   kGUIFontSizeRelative,
   kGUIFontSizePercent,
   kGUIFontSizeEm,
   kGUIFontSizeEx,
   kGUIFontSizePixels,
   kGUIFontSizeInches,
   kGUIFontSizeCentimeters,
   kGUIFontSizeMillimeters,
   kGUIFontSizePoints,
   kGUIFontSizePicas,
};

enum eGUIFontSizeAbsolute
{
   kGUIFontSizeAbsoluteUnspecified,
   kGUIFontXXSmall,
   kGUIFontXSmall,
   kGUIFontSmall,
   kGUIFontMedium,
   kGUIFontLarge,
   kGUIFontXLarge,
   kGUIFontXXLarge
};

enum eGUIFontSizeRelative
{
   kGUIFontSizeRelativeUnspecified,
   kGUIFontSizeLarger,
   kGUIFontSizeSmaller,
};


///////////////////////////////////////////////////////////////////////////////

namespace GUIStandardColors
{
   GUI_API extern const tGUIColor Black;
   GUI_API extern const tGUIColor Red;
   GUI_API extern const tGUIColor Green;
   GUI_API extern const tGUIColor Yellow;
   GUI_API extern const tGUIColor Blue;
   GUI_API extern const tGUIColor Magenta;
   GUI_API extern const tGUIColor Cyan;
   GUI_API extern const tGUIColor DarkGray;
   GUI_API extern const tGUIColor Gray;
   GUI_API extern const tGUIColor LightGray;
   GUI_API extern const tGUIColor White;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUISize
//

template <typename T>
class cGUISize
{
public:
   cGUISize();
   cGUISize(T w, T h);
   cGUISize(const cGUISize & other);
   const cGUISize & operator =(const cGUISize & other);
   bool operator !=(const cGUISize & other) const;
   bool operator ==(const cGUISize & other) const;
   T width, height;
};

///////////////////////////////////////

template <typename T>
inline cGUISize<T>::cGUISize()
{
}

///////////////////////////////////////

template <typename T>
inline cGUISize<T>::cGUISize(T w, T h)
 : width(w), height(h)
{
}

///////////////////////////////////////

template <typename T>
inline cGUISize<T>::cGUISize(const cGUISize & other)
 : width(other.width), height(other.height)
{
}

///////////////////////////////////////

template <typename T>
inline const cGUISize<T> & cGUISize<T>::operator =(const cGUISize & other)
{
   width = other.width;
   height = other.height;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline bool cGUISize<T>::operator !=(const cGUISize & other) const
{
   return (width != other.width) || (height != other.height);
}

///////////////////////////////////////

template <typename T>
inline bool cGUISize<T>::operator ==(const cGUISize & other) const
{
   return (width == other.width) && (height == other.height);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontDesc
//

enum eGUIFontEffects
{
   kGFE_None      = (0<<0),
   kGFE_Bold      = (1<<0),
   kGFE_Italic    = (1<<1),
   kGFE_Shadow    = (1<<2),
   kGFE_Outline   = (1<<3),
};

class GUI_API cGUIFontDesc
{
public:
   enum
   {
      kASCIIGlyphFirst = 32,
      kASCIIGlyphLast = 128,
   };

   cGUIFontDesc();
   cGUIFontDesc(const tGUIChar * pszFace, int pointSize, eGUIFontSizeType sizeType, uint effects,
      uint glyphFirst = kASCIIGlyphFirst, uint glyphLast = kASCIIGlyphLast);
   cGUIFontDesc(const cGUIFontDesc & other);

   const cGUIFontDesc & operator =(const cGUIFontDesc & other);

   bool operator ==(const cGUIFontDesc & other) const;
   bool operator <(const cGUIFontDesc & other) const;

   void SetFace(const tGUIChar * pszFace);
   const tGUIChar * GetFace() const;
   int GetSize() const;
   eGUIFontSizeType GetSizeType() const;
   uint GetEffects() const;
   bool IsBold() const;
   bool IsItalic() const;
   bool IsShadow() const;
   bool IsOutline() const;
   uint GetGlyphFirst() const;
   uint GetGlyphLast() const;

private:
   tGUIString m_typeFace;
   int m_size;
   eGUIFontSizeType m_sizeType;
   uint m_effects;
   uint m_glyphFirst;
   uint m_glyphLast;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUITYPES_H
