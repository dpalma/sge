/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_QUAT_H
#define INCLUDED_QUAT_H

#include "techdll.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template <typename T> class cMatrix4;
typedef class cMatrix4<float> tMatrix4;

typedef class cQuat tQuat;

template <typename T> class cVec3;
typedef class cVec3<float> tVec3;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cQuat
//

class TECH_API cQuat
{
public:
   typedef float value_type;

   cQuat();
   cQuat(const tVec3 & v, value_type _w);
   cQuat(value_type _x, value_type _y, value_type _z, value_type _w);
   cQuat::cQuat(const cQuat & other);

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

inline cQuat::cQuat()
{
}

///////////////////////////////////////

inline cQuat::cQuat(value_type _x, value_type _y, value_type _z, value_type _w)
 : x(_x), y(_y), z(_z), w(_w)
{
}

///////////////////////////////////////

inline cQuat::cQuat(const cQuat & other) 
{
   operator =(other);
}

///////////////////////////////////////

inline const cQuat & cQuat::operator =(const cQuat & other) 
{
   w = other.w;
   x = other.x;
   y = other.y;
   z = other.z;
   return *this;
}

///////////////////////////////////////

inline bool cQuat::operator ==(const cQuat & other) const
{
   return w == other.w && x == other.x && y == other.y && z == other.z;
}

///////////////////////////////////////

inline bool cQuat::operator !=(const cQuat & other) const
{
   return w != other.w || x != other.x || y != other.y || z != other.z;
}

///////////////////////////////////////

inline cQuat::value_type cQuat::Dot(const cQuat & other) const
{
   return (w * other.w) + (x * other.x) + (y * other.y) + (z * other.z);
}

///////////////////////////////////////

inline cQuat::value_type cQuat::Norm() const
{
   return (w * w) + (x * x) + (y * y) + (z * z);
}

///////////////////////////////////////

inline cQuat cQuat::Inverse() const
{
   value_type d = (value_type)1 / Norm();
   return cQuat(-x * d, -y * d, -z * d, w);
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

TECH_API tQuat operator *(const tQuat & q0, const tQuat & q1);

///////////////////////////////////////

TECH_API tQuat QuatFromEulerAngles(const tVec3 & eulerAngles);

TECH_API tQuat QuatSlerp(const tQuat & q0, const tQuat & q1, tQuat::value_type u);

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_QUAT_H
