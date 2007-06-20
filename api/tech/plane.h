///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_PLANE_H
#define INCLUDED_PLANE_H

#include "point3.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cPlane
//

template <typename T>
class cPlane
{
public:
   typedef T value_type;

   cPlane();
   cPlane(value_type aa, value_type bb, value_type cc, value_type dd);
   cPlane(const cVec3<T> & normal, value_type dist);

   void Normalize();

   value_type a, b, c, d;
};

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
cPlane<T>::cPlane(const cVec3<T> & normal, value_type dist)
 : a(normal.x)
 , b(normal.y)
 , c(normal.z)
 , d(dist)
{
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

#ifndef NO_DEFAULT_PLANE
typedef class TECH_API cPlane<float> tPlane;
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_PLANE_H
