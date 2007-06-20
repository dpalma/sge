///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_POINT2_H
#define INCLUDED_POINT2_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPoint2
//

template <typename T>
class cPoint2
{
public:
   typedef T value_type;

   cPoint2();
   cPoint2(value_type xx, value_type yy);
   cPoint2(const cPoint2 & other);

   const cPoint2 & operator =(const cPoint2 & other);
   const cPoint2 & operator +=(const cPoint2 & other);
   const cPoint2 & operator -=(const cPoint2 & other);
   const cPoint2 & operator *=(value_type scale);
   const cPoint2 & operator /=(value_type divisor);

   union
   {
      struct
      {
         value_type x, y;
      };
      value_type xy[2];
   };
};

///////////////////////////////////////

template <typename T>
inline cPoint2<T>::cPoint2()
{
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T>::cPoint2(value_type xx, value_type yy)
: x(xx)
, y(yy)
{
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T>::cPoint2(const cPoint2 & other)
: x(other.x)
, y(other.y)
{
}

///////////////////////////////////////

template <typename T>
inline const cPoint2<T> & cPoint2<T>::operator =(const cPoint2 & other)
{
   x = other.x;
   y = other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cPoint2<T> & cPoint2<T>::operator +=(const cPoint2 & other)
{
   x += other.x;
   y += other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cPoint2<T> & cPoint2<T>::operator -=(const cPoint2 & other)
{
   x -= other.x;
   y -= other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cPoint2<T> & cPoint2<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cPoint2<T> & cPoint2<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T> operator -(const cPoint2<T> & a, const cPoint2<T> & b)
{
   return cPoint2<T>(a.x - b.x, a.y - b.y);
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T> operator +(const cPoint2<T> & a, const cPoint2<T> & b)
{
   return cPoint2<T>(a.x + b.x, a.y + b.y);
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T> operator *(const cPoint2<T> & a, typename cPoint2<T>::value_type scale)
{
   return cPoint2<T>(a.x * scale, a.y * scale);
}

///////////////////////////////////////

template <typename T>
inline cPoint2<T> operator /(const cPoint2<T> & a, typename cPoint2<T>::value_type divisor)
{
   return cPoint2<T>(a.x / divisor, a.y / divisor);
}

///////////////////////////////////////

template <typename T>
inline typename cPoint2<T>::value_type DistanceSqr(const cPoint2<T> & p1, const cPoint2<T> & p2)
{
   return sqr(p2.x - p1.x) + sqr(p2.y - p1.y);
}

///////////////////////////////////////

template <typename T>
inline T Distance(const cPoint2<T> & p1, const cPoint2<T> & p2)
{
   Assert(!"Don't use default version of Distance");
   return 0;
}

template <>
inline double Distance(const cPoint2<double> & p1, const cPoint2<double> & p2)
{
   return sqrt(DistanceSqr(p1, p2));
}

template <>
inline float Distance(const cPoint2<float> & p1, const cPoint2<float> & p2)
{
   return sqrtf(DistanceSqr(p1, p2));
}

///////////////////////////////////////

template <typename T>
inline bool AlmostEqual(const cPoint2<T> & p1, const cPoint2<T> & p2, int maxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cPoint2<float> & p1, const cPoint2<float> & p2,
                        int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cPoint2<double> & p1, const cPoint2<double> & p2,
                        int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_POINT2_H
