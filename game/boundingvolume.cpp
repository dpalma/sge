///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "boundingvolume.h"

#include <cfloat>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBoundingVolume
//

///////////////////////////////////////

cBoundingVolume::cBoundingVolume()
{
}

///////////////////////////////////////

cBoundingVolume::~cBoundingVolume()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cBoundingSphere
//

///////////////////////////////////////

cBoundingSphere::cBoundingSphere()
{
}

///////////////////////////////////////

cBoundingSphere::cBoundingSphere(tVec3 center, tVec3::value_type radius)
 : m_center(center), m_radius(radius)
{
}

///////////////////////////////////////

cBoundingSphere::~cBoundingSphere()
{
}

///////////////////////////////////////

bool cBoundingSphere::Intersects(const tVec3 & origin, const tVec3 & direction) const
{
   // Don't need to calculate the 'a' term because the ray is a unit vector
//   Assert(fabs(1 - direction.LengthSqr()) < FLT_EPSILON);

   // Calculate the 'b' term 
   float b = 2 *((direction.x * (origin.x - GetCenter().x)) +
                 (direction.y * (origin.y - GetCenter().y)) +
                 (direction.z * (origin.z - GetCenter().z)));

   // Calculate the 'c' term 
   float c = sqr(origin.x - GetCenter().x) +
             sqr(origin.y - GetCenter().y) +
             sqr(origin.z - GetCenter().z) -
             sqr(GetRadius());

   // Calculate the discriminant. If it is not positive then the ray 
   // does not hit the sphere. Don't need the 'a' term because the 
   // ray is a unit vector.
   float discr = (sqr(b) - 4*c);
   if (discr <= FLT_EPSILON)
      return false;

   // Solve the quadratic equation. Remember the roots are:
   // (-b +/- sqrt(b^2 - 4*a*c)) / 2a.
   discr = sqrtf(discr);

   // If t0 is positive we are done. If not, we must calculate 
   // the other root.
   float t0 = (-b - discr) * 0.5f;
   if (t0 > -FLT_EPSILON)
   {
      return true;
   }

   float t1 = (-b + discr) * 0.5f;
   if (t1 > -FLT_EPSILON)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
