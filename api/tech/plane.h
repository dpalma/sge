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
   cPlane(const cPoint3<T> & p1, const cPoint3<T> & p2, const cPoint3<T> & p3);

   void Normalize();

   cVec3<T> GetNormal() const;

   value_type a, b, c, d;
};

////////////////////////////////////////

#ifndef NO_DEFAULT_PLANE
typedef class TECH_API cPlane<float> tPlane;
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_PLANE_H
