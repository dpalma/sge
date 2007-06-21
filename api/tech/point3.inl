///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_POINT3_INL
#define INCLUDED_POINT3_INL

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec3;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPoint3
//

///////////////////////////////////////

template <typename T>
inline cPoint3<T>::cPoint3()
{
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T>::cPoint3(value_type xx, value_type yy, value_type zz)
: x(xx)
, y(yy)
, z(zz)
{
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T>::cPoint3(const value_type xyz[3])
: x(xyz[0])
, y(xyz[1])
, z(xyz[2])
{
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T>::cPoint3(const_reference other)
: x(other.x)
, y(other.y)
, z(other.z)
{
}

///////////////////////////////////////

template <typename T>
inline typename cPoint3<T>::const_reference cPoint3<T>::operator =(const_reference other)
{
   x = other.x;
   y = other.y;
   z = other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cPoint3<T>::const_reference cPoint3<T>::operator +=(const_reference other)
{
   x += other.x;
   y += other.y;
   z += other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cPoint3<T>::const_reference cPoint3<T>::operator -=(const_reference other)
{
   x -= other.x;
   y -= other.y;
   z -= other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cPoint3<T>::const_reference cPoint3<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   z *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cPoint3<T>::const_reference cPoint3<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   z /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T> operator -(const cPoint3<T> & a, const cPoint3<T> & b)
{
   return cPoint3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator -(const cPoint3<T> & a, const cPoint3<T> & b)
{
   return cVec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T> operator +(const cPoint3<T> & a, const cPoint3<T> & b)
{
   return cPoint3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T> operator *(const cPoint3<T> & a, typename cPoint3<T>::value_type scale)
{
   return cPoint3<T>(a.x * scale, a.y * scale, a.z * scale);
}

///////////////////////////////////////

template <typename T>
inline cPoint3<T> operator /(const cPoint3<T> & a, typename cPoint3<T>::value_type divisor)
{
   return cPoint3<T>(a.x / divisor, a.y / divisor, a.z / divisor);
}

///////////////////////////////////////

template <typename T>
inline T DistanceSqr(const cPoint3<T> & p1, const cPoint3<T> & p2)
{
   return sqr(p2.x - p1.x) + sqr(p2.y - p1.y) + sqr(p2.z - p1.z);
}

///////////////////////////////////////

template <typename T>
inline T Distance(const cPoint3<T> & p1, const cPoint3<T> & p2)
{
   Assert(!"Don't use default version of Distance");
   return 0;
}

template <>
inline double Distance(const cPoint3<double> & p1, const cPoint3<double> & p2)
{
   return sqrt(DistanceSqr(p1, p2));
}

template <>
inline float Distance(const cPoint3<float> & p1, const cPoint3<float> & p2)
{
   return sqrtf(DistanceSqr(p1, p2));
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cPoint3<T> Lerp(const cPoint3<T> & p1, const cPoint3<T> & p2, T u)
{
   return (p1 * (1 - u)) + (p2 * u);
}

///////////////////////////////////////

template <typename T>
inline bool AlmostEqual(const cPoint3<T> & p1, const cPoint3<T> & p2, int maxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace)
      && AlmostEqual(p1.z, p2.z, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cPoint3<float> & p1, const cPoint3<float> & p2,
                        int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace)
      && AlmostEqual(p1.z, p2.z, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cPoint3<double> & p1, const cPoint3<double> & p2,
                        int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   return AlmostEqual(p1.x, p2.x, maxUnitsLastPlace)
      && AlmostEqual(p1.y, p2.y, maxUnitsLastPlace)
      && AlmostEqual(p1.z, p2.z, maxUnitsLastPlace);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_POINT3_INL

