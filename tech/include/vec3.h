///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_VEC3_H
#define INCLUDED_VEC3_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVec3
//

template <typename T>
class cVec3
{
public:
   typedef T value_type;

   cVec3();
   cVec3(value_type xx, value_type yy, value_type zz);
   cVec3(const value_type v[3]);
   cVec3(const cVec3 & other);

   const cVec3 & operator =(const cVec3 & other);
   const cVec3 & operator +=(const cVec3 & other);
   const cVec3 & operator -=(const cVec3 & other);
   const cVec3 & operator *=(value_type scale);
   const cVec3 & operator /=(value_type divisor);

   value_type Length() const;
   value_type LengthSqr() const;
   void Normalize();
   value_type Dot(const cVec3 & other) const;
   cVec3 Cross(const cVec3 & other) const;

   union
   {
      struct
      {
         value_type x, y, z;
      };
      value_type v[3];
   };
};

///////////////////////////////////////

#ifndef NO_EXPORT_VEC3
#ifndef TECH_EXPORTS
#pragma warning(disable:4231) // nonstandard extension used : 'extern' before template explicit instantiation
extern
#endif
template class TECH_API cVec3<float>;
#endif

///////////////////////////////////////

#ifndef NO_DEFAULT_VEC3
typedef class cVec3<float> tVec3;
#endif

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3()
{
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(value_type xx, value_type yy, value_type zz)
{
   x = xx;
   y = yy;
   z = zz;
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(const value_type v[3])
{
   x = v[0];
   y = v[1];
   z = v[2];
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(const cVec3 & other)
{
   x = other.x;
   y = other.y;
   z = other.z;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cVec3<T>::operator =(const cVec3 & other)
{
   x = other.x;
   y = other.y;
   z = other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cVec3<T>::operator +=(const cVec3 & other)
{
   x += other.x;
   y += other.y;
   z += other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cVec3<T>::operator -=(const cVec3 & other)
{
   x -= other.x;
   y -= other.y;
   z -= other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cVec3<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   z *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline const cVec3<T> & cVec3<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   z /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::value_type cVec3<T>::Length() const
{
   return sqrt(LengthSqr());
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::value_type cVec3<T>::LengthSqr() const
{
   return x * x + y * y + z * z;
}

///////////////////////////////////////

template <typename T>
inline void cVec3<T>::Normalize()
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
inline typename cVec3<T>::value_type cVec3<T>::Dot(const cVec3 & other) const
{
   return x * other.x + y * other.y + z * other.z;
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> cVec3<T>::Cross(const cVec3 & other) const
{
   return cVec3(y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator -(const cVec3<T> & a, const cVec3<T> & b)
{
   return cVec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator +(const cVec3<T> & a, const cVec3<T> & b)
{
   return cVec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator *(const cVec3<T> & a, typename cVec3<T>::value_type scale)
{
   return cVec3<T>(a.x * scale, a.y * scale, a.z * scale);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator /(const cVec3<T> & a, typename cVec3<T>::value_type divisor)
{
   return cVec3<T>(a.x / divisor, a.y / divisor, a.z / divisor);
}

///////////////////////////////////////

template <typename T>
inline T Vec3DistanceSqr(const cVec3<T> & v1, const cVec3<T> & v2)
{
   return sqr(v2.x - v1.x) + sqr(v2.y - v1.y) + sqr(v2.z - v1.z);
}

///////////////////////////////////////

template <typename T>
inline T Vec3Distance(const cVec3<T> & v1, const cVec3<T> & v2)
{
   Assert(!"Don't use default version of Vec3Distance");
   return 0;
}

template <>
inline double Vec3Distance(const cVec3<double> & v1, const cVec3<double> & v2)
{
   return sqrt(Vec3DistanceSqr(v1, v2));
}

template <>
inline float Vec3Distance(const cVec3<float> & v1, const cVec3<float> & v2)
{
   return sqrtf(Vec3DistanceSqr(v1, v2));
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cVec3<T> Vec3Lerp(const cVec3<T> & v1, const cVec3<T> & v2, T u)
{
   return (v1 * (1 - u)) + (v2 * u);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_VEC3_H

