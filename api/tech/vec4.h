///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_VEC4_H
#define INCLUDED_VEC4_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVec4
//

template <typename T>
class cVec4
{
public:
   typedef T value_type;

   cVec4();
   cVec4(value_type xx, value_type yy, value_type zz, value_type ww);
   cVec4(const cVec4 & other);

   const cVec4 & operator =(const cVec4 & other);
   const cVec4 & operator +=(const cVec4 & other);
   const cVec4 & operator -=(const cVec4 & other);
   const cVec4 & operator *=(value_type scale);
   const cVec4 & operator /=(value_type divisor);

   value_type Length() const;
   value_type LengthSqr() const;
   void Normalize();
   value_type Dot(const cVec4 & other);

   union
   {
      struct
      {
         value_type x, y, z, w;
      };
      value_type v[4];
   };
};

///////////////////////////////////////

template <typename T>
inline cVec4<T>::cVec4()
{
}

///////////////////////////////////////

template <typename T>
inline cVec4<T>::cVec4(value_type xx, value_type yy, value_type zz, value_type ww)
{
   x = xx;
   y = yy;
   z = zz;
   w = ww;
}

///////////////////////////////////////

template <typename T>
inline cVec4<T>::cVec4(const cVec4 & other)
{
   x = other.x;
   y = other.y;
   z = other.z;
   w = other.w;
}

///////////////////////////////////////

template <typename T>
inline const cVec4<T> & cVec4<T>::operator =(const cVec4 & other)
{
   x = other.x;
   y = other.y;
   z = other.z;
   w = other.w;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec4<T> & cVec4<T>::operator +=(const cVec4 & other)
{
   x += other.x;
   y += other.y;
   z += other.z;
   w += other.w;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec4<T> & cVec4<T>::operator -=(const cVec4 & other)
{
   x -= other.x;
   y -= other.y;
   z -= other.z;
   w -= other.w;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec4<T> & cVec4<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   z *= scale;
   w *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec4<T> & cVec4<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   z /= divisor;
   w /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec4<T>::value_type cVec4<T>::Length() const
{
   return sqrt(LengthSqr());
}

template <>
inline float cVec4<float>::Length() const
{
   return sqrtf(LengthSqr());
}

///////////////////////////////////////

template <typename T>
inline typename cVec4<T>::value_type cVec4<T>::LengthSqr() const
{
   return x * x + y * y + z * z + w * w;
}

///////////////////////////////////////

template <typename T>
inline void cVec4<T>::Normalize()
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
inline typename cVec4<T>::value_type cVec4<T>::Dot(const cVec4<T> & other)
{
   return x * other.x + y * other.y + z * other.z + w * other.w;
}

///////////////////////////////////////

template <typename T>
inline cVec4<T> operator -(const cVec4<T> & a, const cVec4<T> & b)
{
   return cVec4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

///////////////////////////////////////

template <typename T>
inline cVec4<T> operator +(const cVec4<T> & a, const cVec4<T> & b)
{
   return cVec4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

///////////////////////////////////////

template <typename T>
inline cVec4<T> operator *(const cVec4<T> & a, typename cVec4<T>::value_type scale)
{
   return cVec4<T>(a.x * scale, a.y * scale, a.z * scale, a.w * scale);
}

///////////////////////////////////////

template <typename T>
inline cVec4<T> operator /(const cVec4<T> & a, typename cVec4<T>::value_type divisor)
{
   return cVec4<T>(a.x / divisor, a.y / divisor, a.z / divisor, a.w / divisor);
}

///////////////////////////////////////

template <typename T>
inline T Dot(const cVec4<T> & a, const cVec4<T> & b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

///////////////////////////////////////

template <typename T>
inline bool AlmostEqual(const cVec4<T> & v1, const cVec4<T> & v2, int maxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace)
      && AlmostEqual(v1.w, v2.w, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec4<float> & v1, const cVec4<float> & v2,
                        int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace)
      && AlmostEqual(v1.w, v2.w, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec4<double> & v1, const cVec4<double> & v2,
                        int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace)
      && AlmostEqual(v1.w, v2.w, maxUnitsLastPlace);
}

///////////////////////////////////////

#ifndef NO_DEFAULT_VEC4
typedef class TECH_API cVec4<float> tVec4;
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_VEC4_H

