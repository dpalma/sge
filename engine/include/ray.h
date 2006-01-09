///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RAY_H
#define INCLUDED_RAY_H

#include "enginedll.h"

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

class cAxisAlignedBox;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRay
//

class ENGINE_API cRay
{
public:
   cRay();
   cRay(const tVec3 & origin, const tVec3 & direction);
   cRay(const cRay & ray);
   const cRay & operator =(const cRay & ray);

   const tVec3 & GetOrigin() const;
   const tVec3 & GetDirection() const;

   bool IntersectsSphere(const tVec3 & center, 
                         tVec3::value_type radius, 
                         tVec3 * pIntersection = NULL) const;

   bool IntersectsPlane(const tVec3 & normal, 
                        tVec3::value_type d,
                        tVec3 * pIntersection = NULL) const;

   bool IntersectsTriangle(const tVec3 & v1,
                           const tVec3 & v2,
                           const tVec3 & v3,
                           tVec3 * pIntersection = NULL) const;

   bool IntersectsAxisAlignedBox(const cAxisAlignedBox & box,
                                 float * pTNear = NULL,
                                 float * pTFar = NULL) const;

private:
   tVec3 m_origin, m_direction;
};

///////////////////////////////////////

inline const tVec3 & cRay::GetOrigin() const
{
   return m_origin;
}

///////////////////////////////////////

inline const tVec3 & cRay::GetDirection() const
{
   return m_direction;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RAY_H
