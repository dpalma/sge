///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_UITYPES_H
#define INCLUDED_UITYPES_H

#include "color.h"
#include "str.h"
#include "vec2.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

typedef cStr cUIString;

template <typename T> class cVec2;
typedef class cVec2<float> tVec2;

typedef tVec2 cUIPoint;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUISize
//

struct sUISize
{
   float width, height;
};

///////////////////////////////////////

class cUISize : public sUISize
{
public:
   cUISize();
   cUISize(float w, float h);
   cUISize(const cUISize & other);
   const cUISize & operator =(const cUISize & other);
   bool operator !=(const cUISize & other) const;
   bool operator ==(const cUISize & other) const;
};

///////////////////////////////////////

inline cUISize::cUISize()
{
}

///////////////////////////////////////

inline cUISize::cUISize(float w, float h)
{
   width = w;
   height = h;
}

///////////////////////////////////////

inline cUISize::cUISize(const cUISize & other)
{
   operator=(other);
}

///////////////////////////////////////

inline const cUISize & cUISize::operator =(const cUISize & other)
{
   width = other.width;
   height = other.height;
   return *this;
}

///////////////////////////////////////

inline bool cUISize::operator !=(const cUISize & other) const
{
   return width != other.width || height != other.height;
}

///////////////////////////////////////

inline bool cUISize::operator ==(const cUISize & other) const
{
   return width == other.width && height == other.height;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIRect
//

struct sUIRect
{
   float left, top, right, bottom;
};

///////////////////////////////////////

class cUIRect : public sUIRect
{
public:
   cUIRect();
   cUIRect(float l, float t, float r, float b);
   cUIRect(const cUIPoint & topLeft, const cUISize & size);
   bool PtInside(const cUIPoint & point) const;
   void Offset(float x, float y);
   float GetWidth() const;
   float GetHeight() const;
   cUIPoint GetTopLeft() const;
};

///////////////////////////////////////

inline cUIRect::cUIRect()
{
}

///////////////////////////////////////

inline cUIRect::cUIRect(float l, float t, float r, float b)
{
   left = l;
   top = t;
   right = r;
   bottom = b;
}

///////////////////////////////////////

inline cUIRect::cUIRect(const cUIPoint & topLeft, const cUISize & size)
{
   left = topLeft.x;
   top = topLeft.y;
   right = topLeft.x + size.width;
   bottom = topLeft.y + size.height;
}

///////////////////////////////////////

inline bool cUIRect::PtInside(const cUIPoint & point) const
{
   if (point.x >= left && point.x <= right &&
       point.y >= top && point.y <= bottom)
      return true;
   else
      return false;
}

///////////////////////////////////////

inline void cUIRect::Offset(float x, float y)
{
   left += x;
   top += y;
   right += x;
   bottom += y;
}

///////////////////////////////////////

inline float cUIRect::GetWidth() const
{
   return right - left;
}

///////////////////////////////////////

inline float cUIRect::GetHeight() const
{
   return bottom - top;
}

///////////////////////////////////////

inline cUIPoint cUIRect::GetTopLeft() const
{
   return cUIPoint(left, top);
}


///////////////////////////////////////////////////////////////////////////////

typedef cUIRect cUIMargins;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIColor4f
//

class cUIColor4f : public cColor
{
public:
   cUIColor4f() : cColor(0,0,0,1) {}
   cUIColor4f(float r, float g, float b, float a) : cColor(r, g, b, a) {}

#ifndef NDEBUG
   // These methods are used only in unit tests
   float GetRed() const { return GetPointer()[0]; }
   float GetGreen() const { return GetPointer()[1]; }
   float GetBlue() const { return GetPointer()[2]; }
   float GetAlpha() const { return GetPointer()[3]; }
#endif
};

typedef cUIColor4f cUIColor;

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_UITYPES_H
