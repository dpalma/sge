///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUITYPES_H
#define INCLUDED_GUITYPES_H

#include "color.h"
#include "str.h"
#include "vec2.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef cStr tGUIString;

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

typedef tVec2 tGUIPoint;

template <typename T> class cGUISize;
typedef class cGUISize<float> tGUISize;

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

class cGUIColor : public cColor
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
