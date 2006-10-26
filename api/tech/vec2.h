///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_VEC2_H
#define INCLUDED_VEC2_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVec2
//

template <typename T>
class cVec2
{
public:
   typedef T value_type;

   cVec2();
   cVec2(value_type xx, value_type yy);
   cVec2(const cVec2 & other);

   const cVec2 & operator =(const cVec2 & other);
   const cVec2 & operator +=(const cVec2 & other);
   const cVec2 & operator -=(const cVec2 & other);
   const cVec2 & operator *=(value_type scale);
   const cVec2 & operator /=(value_type divisor);

   value_type Length() const;
   value_type LengthSqr() const;
   void Normalize();
   value_type Dot(const cVec2 & other);

   union
   {
      struct
      {
         value_type x, y;
      };
      value_type v[2];
   };
};

///////////////////////////////////////

template <typename T>
inline cVec2<T>::cVec2()
{
}

///////////////////////////////////////

template <typename T>
inline cVec2<T>::cVec2(value_type xx, value_type yy)
{
   x = xx;
   y = yy;
}

///////////////////////////////////////

template <typename T>
inline cVec2<T>::cVec2(const cVec2 & other)
{
   x = other.x;
   y = other.y;
}

///////////////////////////////////////

template <typename T>
inline const cVec2<T> & cVec2<T>::operator =(const cVec2 & other)
{
   x = other.x;
   y = other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec2<T> & cVec2<T>::operator +=(const cVec2 & other)
{
   x += other.x;
   y += other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec2<T> & cVec2<T>::operator -=(const cVec2 & other)
{
   x -= other.x;
   y -= other.y;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec2<T> & cVec2<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec2<T> & cVec2<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec2<T>::value_type cVec2<T>::Length() const
{
   return sqrt(LengthSqr());
}

template <>
inline float cVec2<float>::Length() const
{
   return sqrtf(LengthSqr());
}

///////////////////////////////////////

template <typename T>
inline typename cVec2<T>::value_type cVec2<T>::LengthSqr() const
{
   return x * x + y * y;
}

///////////////////////////////////////

template <typename T>
inline void cVec2<T>::Normalize()
{
   value_type m = Length();
   if (m > 0)
      m = 1.0f / m;
   else
      m = 0;
   operator *=(m);
}

///////////////////////////////////////

template <typename T>
inline typename cVec2<T>::value_type cVec2<T>::Dot(const cVec2 & other)
{
   return x * other.x + y * other.y;
}

///////////////////////////////////////

template <typename T>
inline cVec2<T> operator -(const cVec2<T> & a, const cVec2<T> & b)
{
   return cVec2<T>(a.x - b.x, a.y - b.y);
}

///////////////////////////////////////

template <typename T>
inline cVec2<T> operator +(const cVec2<T> & a, const cVec2<T> & b)
{
   return cVec2<T>(a.x + b.x, a.y + b.y);
}

///////////////////////////////////////

template <typename T>
inline cVec2<T> operator *(const cVec2<T> & a, typename cVec2<T>::value_type scale)
{
   return cVec2<T>(a.x * scale, a.y * scale);
}

///////////////////////////////////////

template <typename T>
inline cVec2<T> operator /(const cVec2<T> & a, typename cVec2<T>::value_type divisor)
{
   return cVec2<T>(a.x / divisor, a.y / divisor);
}

///////////////////////////////////////

template <typename T>
inline typename cVec2<T>::value_type Vec2DistanceSqr(const cVec2<T> & v1, const cVec2<T> & v2)
{
   return sqr(v2.x - v1.x) + sqr(v2.y - v1.y);
}

///////////////////////////////////////

template <typename T>
inline T Vec2Distance(const cVec2<T> & v1, const cVec2<T> & v2)
{
   Assert(!"Don't use default version of Vec2Distance");
   return 0;
}

template <>
inline double Vec2Distance(const cVec2<double> & v1, const cVec2<double> & v2)
{
   return sqrt(Vec2DistanceSqr(v1, v2));
}

template <>
inline float Vec2Distance(const cVec2<float> & v1, const cVec2<float> & v2)
{
   return sqrtf(Vec2DistanceSqr(v1, v2));
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cVec2<T> Vec2Lerp(const cVec2<T> & v1, const cVec2<T> & v2, T u)
{
   return (v1 * (1 - u)) + (v2 * u);
}

///////////////////////////////////////

template <typename T>
inline bool AlmostEqual(const cVec2<T> & v1, const cVec2<T> & v2, int maxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec2<float> & v1, const cVec2<float> & v2,
                        int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec2<double> & v1, const cVec2<double> & v2,
                        int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace);
}

///////////////////////////////////////

#ifndef NO_DEFAULT_VEC2
typedef class TECH_API cVec2<float> tVec2;
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_VEC2_H

