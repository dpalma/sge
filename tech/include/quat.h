/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_QUAT_H
#define INCLUDED_QUAT_H

#include "techdll.h"
#include "matrix4.h"
#include "vec3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

#ifndef NO_DEFAULT_QUAT
template <typename T> class cQuat;
typedef cQuat<float> tQuat;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cQuat
//

template <typename T>
class cQuat
{
public:
   typedef T value_type;

   cQuat();
   cQuat(const tVec3 & v, value_type _w);
   cQuat(value_type _x, value_type _y, value_type _z, value_type _w);
   cQuat(const cQuat & other);

   const cQuat & operator =(const cQuat & other);

   bool operator ==(const cQuat & other) const;
   bool operator !=(const cQuat & other) const;

   value_type Dot(const cQuat & quat) const;
   value_type Norm() const;
   cQuat Inverse() const;

   void ToMatrix(tMatrix4 * pMatrix) const;

   value_type x, y, z, w;
};

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat()
{
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(const tVec3 & v, value_type _w)
 : x(v.x), y(v.y), z(v.z), w(_w)
{
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(value_type _x, value_type _y, value_type _z, value_type _w)
 : x(_x), y(_y), z(_z), w(_w)
{
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(const cQuat & other) 
{
   operator =(other);
}

///////////////////////////////////////

template <typename T>
inline const cQuat<T> & cQuat<T>::operator =(const cQuat & other) 
{
   w = other.w;
   x = other.x;
   y = other.y;
   z = other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline bool cQuat<T>::operator ==(const cQuat & other) const
{
   return w == other.w && x == other.x && y == other.y && z == other.z;
}

///////////////////////////////////////

template <typename T>
inline bool cQuat<T>::operator !=(const cQuat & other) const
{
   return w != other.w || x != other.x || y != other.y || z != other.z;
}

///////////////////////////////////////

template <typename T>
inline T cQuat<T>::Dot(const cQuat & other) const
{
   return (w * other.w) + (x * other.x) + (y * other.y) + (z * other.z);
}

///////////////////////////////////////

template <typename T>
inline T cQuat<T>::Norm() const
{
   return (w * w) + (x * x) + (y * y) + (z * z);
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> cQuat<T>::Inverse() const
{
   value_type d = (value_type)1 / Norm();
   return cQuat(-x * d, -y * d, -z * d, w);
}

///////////////////////////////////////

template <typename T>
inline void cQuat<T>::ToMatrix(tMatrix4 * pMatrix) const
{
   tQuat::value_type s = 2.0f / Norm();
   tQuat::value_type xs = x*s;
   tQuat::value_type ys = y*s;
   tQuat::value_type zs = z*s;

   tQuat::value_type wx = w*xs;
   tQuat::value_type wy = w*ys;
   tQuat::value_type wz = w*zs;

   tQuat::value_type xx = x*xs;
   tQuat::value_type xy = x*ys;
   tQuat::value_type xz = x*zs;

   tQuat::value_type yy = y*ys;
   tQuat::value_type yz = y*zs;
   tQuat::value_type zz = z*zs;

   pMatrix->m[0] = 1 - (yy + zz);
   pMatrix->m[1] = xy + wz;
   pMatrix->m[2] = xz - wy;
   pMatrix->m[3] = 0;

   pMatrix->m[4] = xy - wz;
   pMatrix->m[5] = 1 - (xx + zz);
   pMatrix->m[6] = yz + wx;
   pMatrix->m[7] = 0;

   pMatrix->m[8] = xz + wy;
   pMatrix->m[9] = yz - wx;
   pMatrix->m[10] = 1 - (xx + yy);
   pMatrix->m[11] = 0;

   pMatrix->m[12] = 0;
   pMatrix->m[13] = 0;
   pMatrix->m[14] = 0;
   pMatrix->m[15] = 1;
}

///////////////////////////////////////

inline tQuat operator +(const tQuat & a, const tQuat & b)
{
   return tQuat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

///////////////////////////////////////

inline tQuat operator -(const tQuat & a, const tQuat & b)
{
   return tQuat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

///////////////////////////////////////

inline tQuat operator *(const tQuat & q, tQuat::value_type scalar)
{
   return tQuat(q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar);
}

///////////////////////////////////////

inline tQuat operator /(const tQuat & q, tQuat::value_type scalar)
{
   tQuat::value_type oneOver = (tQuat::value_type)1 / scalar;
   return tQuat(q.x * oneOver, q.y * oneOver, q.z * oneOver, q.w * oneOver);
}

///////////////////////////////////////

tQuat operator *(const tQuat & q0, const tQuat & q1);

///////////////////////////////////////

tQuat QuatFromEulerAngles(const tVec3 & eulerAngles);

tQuat QuatSlerp(const tQuat & q0, const tQuat & q1, tQuat::value_type u);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_QUAT_H
