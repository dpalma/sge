/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_QUAT_INL
#define INCLUDED_QUAT_INL

#include "matrix3.h"
#include "techmath.h"
#include "vec3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// REFERENCES
// "Rotating Objects Using Quaternions", February, 1998 "Game Developer", pp. 34-42
// "Quaternions for Game Programming", "Game Programming Gems", Section 2.7

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cQuat
//

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat()
{
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(const cVec3<T> & axis, T angle)
{
   T halfAngle = angle / 2;
   T s = sin(halfAngle);
   T c = cos(halfAngle);
   x = s * axis.x;
   y = s * axis.y;
   z = s * axis.z;
   w = c;
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(value_type xx, value_type yy, value_type zz, value_type ww)
 : x(xx)
 , y(yy)
 , z(zz)
 , w(ww)
{
}

///////////////////////////////////////

template <typename T>
inline cQuat<T>::cQuat(const cQuat & other) 
 : x(other.x)
 , y(other.y)
 , z(other.z)
 , w(other.w)
{
}

///////////////////////////////////////

template <typename T>
inline const cQuat<T> & cQuat<T>::operator =(const cQuat & other) 
{
   Assign(other);
   return *this;
}

///////////////////////////////////////

template <typename T>
inline void cQuat<T>::Assign(const cQuat & other) 
{
   w = other.w;
   x = other.x;
   y = other.y;
   z = other.z;
}

///////////////////////////////////////

template <typename T>
const cQuat<T> & cQuat<T>::GetMultIdentity()
{
   static const cQuat<T> multIdentity(0,0,0,1);
   return multIdentity;
}

///////////////////////////////////////

template <typename T>
inline bool cQuat<T>::EqualTo(const cQuat & other) const
{
   return w == other.w && x == other.x && y == other.y && z == other.z;
}

///////////////////////////////////////

template <typename T>
const cQuat<T> & cQuat<T>::operator *=(const cQuat & other)
{
   T wNew = ((w * other.w) - (x * other.x) - (y * other.y) - (z * other.z));
   T xNew = ((w * other.x) + (x * other.w) + (y * other.z) - (z * other.y));
   T yNew = ((w * other.y) - (x * other.z) + (y * other.w) + (z * other.x));
   T zNew = ((w * other.z) + (x * other.y) - (y * other.x) + (z * other.w));
   w = wNew;
   x = xNew;
   y = yNew;
   z = zNew;
   return *this;
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
   T n = Norm();
   Assert(n != 0);
   T oneOverN = static_cast<T>(1) / n;
   return cQuat(-x * oneOverN, -y * oneOverN, -z * oneOverN, w);
}

///////////////////////////////////////

template <typename T>
inline void cQuat<T>::ToMatrix(cMatrix3<T> * pMatrix) const
{
   Assert(pMatrix != NULL);
   if (pMatrix == NULL)
      return;

   value_type s = 2.0f / Norm();
   value_type xs = x*s;
   value_type ys = y*s;
   value_type zs = z*s;

   value_type wx = w*xs;
   value_type wy = w*ys;
   value_type wz = w*zs;

   value_type xx = x*xs;
   value_type xy = x*ys;
   value_type xz = x*zs;

   value_type yy = y*ys;
   value_type yz = y*zs;
   value_type zz = z*zs;

   pMatrix->m[0] = 1 - (yy + zz);
   pMatrix->m[1] = xy + wz;
   pMatrix->m[2] = xz - wy;

   pMatrix->m[3] = xy - wz;
   pMatrix->m[4] = 1 - (xx + zz);
   pMatrix->m[5] = yz + wx;

   pMatrix->m[6] = xz + wy;
   pMatrix->m[7] = yz - wx;
   pMatrix->m[8] = 1 - (xx + yy);
}

///////////////////////////////////////

template <typename T>
cQuat<T> cQuat<T>::FromEulerAngles(T pitch, T yaw, T roll)
{
   cQuat<T> i(cVec3<T>(1, 0, 0), pitch);
   cQuat<T> j(cVec3<T>(0, 1, 0), yaw);
   cQuat<T> k(cVec3<T>(0, 0, 1), roll);
   return cQuat<T>(i * (j * k));
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> operator +(const cQuat<T> & a, const cQuat<T> & b)
{
   return cQuat<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> operator -(const cQuat<T> & a, const cQuat<T> & b)
{
   return cQuat<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> operator *(const cQuat<T> & q, typename cQuat<T>::value_type scalar)
{
   return cQuat<T>(q.x * scalar, q.y * scalar, q.z * scalar, q.w * scalar);
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> operator /(const cQuat<T> & q, typename cQuat<T>::value_type scalar)
{
   cQuat<T>::value_type oneOver = (cQuat<T>::value_type)1 / scalar;
   return cQuat<T>(q.x * oneOver, q.y * oneOver, q.z * oneOver, q.w * oneOver);
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> operator *(const cQuat<T> & q0, const cQuat<T> & q1)
{
   return cQuat<T>(q0) *= q1;
}

///////////////////////////////////////

template <typename T>
inline cQuat<T> Combine(const cQuat<T> & q1, T a, const cQuat<T> & q2, T b)
{
   return cQuat<T>((q1 * a) + (q2 * b));
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cQuat<T> Lerp(const cQuat<T> & q1, const cQuat<T> & q2, T u)
{
   return Combine(q1, 1 - u, q2, u);
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cQuat<T> Slerp(const cQuat<T> & q0, const cQuat<T> & q2, T u, T threshold = .001)
{
   T cosTheta = q0.Dot(q2);

   cQuat<T> q2prime;

   if (cosTheta < 0)
   {
      cosTheta = -cosTheta;
      q2prime = q2.Inverse();
   }
   else
   {
      q2prime = q2;
   }

   T scale0, scale1;

   if (1 - cosTheta > threshold)
   {
      T theta = acosf(cosTheta);

      T oneOverSinTheta = 1.0f / sqrtf(1 - (cosTheta * cosTheta));

      scale0 = sinf((1.0f - u) * theta) * oneOverSinTheta;
      scale1 = sinf(u * theta) * oneOverSinTheta;
   }
   else
   {
      scale0 = (1.0f - u);
      scale1 = u;
   }

   return Combine(q0, scale0, q2prime, scale1);
}

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_QUAT_INL
