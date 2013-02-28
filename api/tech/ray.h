///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RAY_H
#define INCLUDED_RAY_H

#include "axisalignedbox.h"
#include "point3.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRay
//

template <typename T>
class cRay
{
public:
   cRay();
   cRay(const cPoint3<T> & origin, const cVec3<T> & direction);
   cRay(const cRay & ray);
   const cRay & operator =(const cRay & ray);

   const cPoint3<T> & GetOrigin() const;
   const cVec3<T> & GetDirection() const;

   bool IntersectsSphere(const cPoint3<T> & center, 
                         typename cVec3<T>::value_type radius, 
                         cPoint3<T> * pIntersection = NULL) const;

   bool IntersectsPlane(const cVec3<T> & normal, 
                        typename cVec3<T>::value_type d,
                        cPoint3<T> * pIntersection = NULL) const;

   bool IntersectsTriangle(const cPoint3<T> & v1,
                           const cPoint3<T> & v2,
                           const cPoint3<T> & v3,
                           cPoint3<T> * pIntersection = NULL) const;

   bool IntersectsAxisAlignedBox(const cAxisAlignedBox<T> & box,
                                 T * pTNear = NULL,
                                 T * pTFar = NULL) const;

private:
   cPoint3<T> m_origin;
   cVec3<T> m_direction;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RAY_H
