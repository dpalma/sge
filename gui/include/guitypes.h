///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITYPES_H
#define INCLUDED_GUITYPES_H

#include "enginedll.h"

#include "color.h"
#include "str.h"
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

#endif // INCLUDED_GUITYPES_H
