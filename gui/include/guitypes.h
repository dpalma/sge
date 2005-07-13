///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITYPES_H
#define INCLUDED_GUITYPES_H

#include "enginedll.h"

#include "color.h"
#include "techstring.h"
#include "vec2.h"
#include "rect.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef cStr tGUIString;

typedef tVec2 tGUIPoint;

template <typename T> class cGUISize;
typedef float tGUISizeType;
typedef class cGUISize<tGUISizeType> tGUISize;

typedef tRect tGUIRect;


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sGUIInsets
//

struct sGUIInsets
{
   int left, top, right, bottom;
};

typedef struct sGUIInsets tGUIInsets;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIColor
//

#pragma warning(disable:4275) // non dll-interface class 'X' used as base for dll-interface class 'Y'

class ENGINE_API cGUIColor : public cColor
{
public:
   cGUIColor() : cColor() {}
   cGUIColor(float r, float g, float b) : cColor(r, g, b) {}
   cGUIColor(float r, float g, float b, float a) : cColor(r, g, b, a) {}
   cGUIColor(const float rgba[4]) : cColor(rgba) {}
   cGUIColor(const cGUIColor & other) : cColor(other) {}

   static const cGUIColor Black;
   static const cGUIColor Red;
   static const cGUIColor Green;
   static const cGUIColor Yellow;
   static const cGUIColor Blue;
   static const cGUIColor Magenta;
   static const cGUIColor Cyan;
   static const cGUIColor DarkGray;
   static const cGUIColor Gray;
   static const cGUIColor LightGray;
   static const cGUIColor White;
};

typedef cGUIColor tGUIColor;


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

class ENGINE_API cGUIFontDesc
{
public:
   enum
   {
      kANSIGlyphFirst = 32,
      kANSIGlyphLast = 128,
   };

   cGUIFontDesc();
   cGUIFontDesc(const tChar * pszFace, int pointSize, uint effects,
      uint glyphFirst = kANSIGlyphFirst, uint glyphLast = kANSIGlyphLast);
   cGUIFontDesc(const cGUIFontDesc & other);

   const cGUIFontDesc & operator =(const cGUIFontDesc & other);

   const tChar * GetFace() const;
   int GetPointSize() const;
   bool GetBold() const;
   bool GetItalic() const;
   bool GetShadow() const;
   bool GetOutline() const;
   uint GetGlyphFirst() const;
   uint GetGlyphLast() const;

private:
   cStr m_typeFace;
   int m_pointSize;
   uint m_effects;
   uint m_glyphFirst;
   uint m_glyphLast;
};

////////////////////////////////////////

inline cGUIFontDesc::cGUIFontDesc()
 : m_pointSize(0),
   m_effects(0),
   m_glyphFirst(0),
   m_glyphLast(0)
{
}

////////////////////////////////////////

inline cGUIFontDesc::cGUIFontDesc(const tChar * pszFace, int pointSize, uint effects, uint glyphFirst, uint glyphLast)
 : m_typeFace(pszFace),
   m_pointSize(pointSize),
   m_effects(effects),
   m_glyphFirst(glyphFirst),
   m_glyphLast(glyphLast)
{
}

////////////////////////////////////////

inline cGUIFontDesc::cGUIFontDesc(const cGUIFontDesc & other)
 : m_typeFace(other.m_typeFace),
   m_pointSize(other.m_pointSize),
   m_glyphFirst(other.m_glyphFirst),
   m_glyphLast(other.m_glyphLast)
{
}

////////////////////////////////////////

inline const cGUIFontDesc & cGUIFontDesc::operator =(const cGUIFontDesc & other)
{
   m_typeFace = other.m_typeFace;
   m_pointSize = other.m_pointSize;
   m_glyphFirst = other.m_glyphFirst;
   m_glyphLast = other.m_glyphLast;
   return *this;
}

////////////////////////////////////////

inline const tChar * cGUIFontDesc::GetFace() const
{
   return m_typeFace.c_str();
}

////////////////////////////////////////

inline int cGUIFontDesc::GetPointSize() const
{
   return m_pointSize;
}

////////////////////////////////////////

inline bool cGUIFontDesc::GetBold() const
{
   return (m_effects & kGFE_Bold) == kGFE_Bold;
}

////////////////////////////////////////

inline bool cGUIFontDesc::GetItalic() const
{
   return (m_effects & kGFE_Italic) == kGFE_Italic;
}

////////////////////////////////////////

inline bool cGUIFontDesc::GetShadow() const
{
   return (m_effects & kGFE_Shadow) == kGFE_Shadow;
}

////////////////////////////////////////

inline bool cGUIFontDesc::GetOutline() const
{
   return (m_effects & kGFE_Outline) == kGFE_Outline;
}

////////////////////////////////////////

inline uint cGUIFontDesc::GetGlyphFirst() const
{
   return m_glyphFirst;
}

////////////////////////////////////////

inline uint cGUIFontDesc::GetGlyphLast() const
{
   return m_glyphLast;
}

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_GUITYPES_H
