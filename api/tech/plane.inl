///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_PLANE_INL
#define INCLUDED_PLANE_INL

#include "point3.inl"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cPlane
//

////////////////////////////////////////

template <typename T>
cPlane<T>::cPlane()
{
}

////////////////////////////////////////

template <typename T>
cPlane<T>::cPlane(value_type aa, value_type bb, value_type cc, value_type dd)
 : a(aa)
 , b(bb)
 , c(cc)
 , d(dd)
{
}

////////////////////////////////////////

template <typename T>
cPlane<T>::cPlane(const cPoint3<T> & p1, const cPoint3<T> & p2, const cPoint3<T> & p3)
{
   cVec3<T> n(cVec3<T>(p2 - p1).Cross(cVec3<T>(p3 - p1)));
   n.Normalize();
   a = normal.x;
   b = normal.y;
   c = normal.z;
   d = n.Dot(cVec3<T>(-p1.x, -p1.y, -p1.z));
}

////////////////////////////////////////

template <typename T>
void cPlane<T>::Normalize()
{
   double t = 1.0 / sqrt(a * a + b * b + c * c);
   a *= t;
   b *= t;
   c *= t;
   d *= t;
}

template <>
void cPlane<float>::Normalize()
{
   float t = 1.0f / sqrtf(a * a + b * b + c * c);
   a *= t;
   b *= t;
   c *= t;
   d *= t;
}

////////////////////////////////////////

template <typename T>
cVec3<T> cPlane<T>::GetNormal() const
{
   return cVec3<T>(a, b, c);
}

////////////////////////////////////////

#ifndef NO_DEFAULT_PLANE
typedef class TECH_API cPlane<float> tPlane;
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_PLANE_INL
