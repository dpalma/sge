///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_BOUNDINGVOLUME_H
#define INCLUDED_BOUNDINGVOLUME_H

#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBoundingVolume
//

class cBoundingVolume
{
public:
   cBoundingVolume();
   virtual ~cBoundingVolume() = 0;

   // ray intersection
   virtual bool Intersects(const tVec3 & origin, const tVec3 & direction) const = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBoundingSphere
//

class cBoundingSphere : public cBoundingVolume
{
public:
   cBoundingSphere();
   cBoundingSphere(tVec3 center, tVec3::value_type radius);
   virtual ~cBoundingSphere();

   const tVec3 & GetCenter() const;
   void SetCenter(tVec3 center);

   tVec3::value_type GetRadius() const;
   void SetRadius(tVec3::value_type radius);

   virtual bool Intersects(const tVec3 & origin, const tVec3 & direction) const;

private:
   tVec3 m_center;
   tVec3::value_type m_radius;
};

///////////////////////////////////////

inline const tVec3 & cBoundingSphere::GetCenter() const
{
   return m_center;
}

///////////////////////////////////////

inline void cBoundingSphere::SetCenter(tVec3 center)
{
   m_center = center;
}

///////////////////////////////////////

inline tVec3::value_type cBoundingSphere::GetRadius() const
{
   return m_radius;
}

///////////////////////////////////////

inline void cBoundingSphere::SetRadius(tVec3::value_type radius)
{
   m_radius = radius;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_BOUNDINGVOLUME_H
