///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITYPES_H
#define INCLUDED_GUITYPES_H

#include "guidll.h"

#include "tech/color.h"
#include "tech/rect.h"
#include "tech/techstring.h"
#include "tech/vec2.h"

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

#endif // INCLUDED_GUITYPES_H
