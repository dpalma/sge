///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "raycast.h"
#include "vec3.h"
#include <cfloat>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool RayIntersectSphere(const tVec3 & rayOrigin, const tVec3 & rayDir,
                        const tVec3 & sphereCenter, float sphereRadius,
                        tVec3 * pIntersect)
{
   // Don't need to calculate the 'a' term because the ray is a unit vector

   // Calculate the 'b' term 
   float b = 2 *((rayDir.x * (rayOrigin.x - sphereCenter.x)) +
                 (rayDir.y * (rayOrigin.y - sphereCenter.y)) +
                 (rayDir.z * (rayOrigin.z - sphereCenter.z)));

   // Calculate the 'c' term 
   float c = sqr(rayOrigin.x - sphereCenter.x) +
             sqr(rayOrigin.y - sphereCenter.y) +
             sqr(rayOrigin.z - sphereCenter.z) -
             sqr(sphereRadius);

   // Calculate the discriminant. If discriminant not positive
   // then the ray does not hit the sphere. Return a t of 0.
   float discr = (sqr(b) - 4*c);
   if (discr <= FLT_EPSILON)
      return false;

   // Solve the quadratic equation. You remember the roots are:
   // (-b +/- sqrt(b^2 - 4*a*c)) / 2a.
   discr = sqrtf(discr);
   float t0 = (-b - discr) * 0.5f;

   // If t0 is positive we are done. If not, we must calculate
   // the other root t1.
   if (t0 > -FLT_EPSILON)
   {
      if (pIntersect != NULL)
      {
         pIntersect->x = rayOrigin.x + (rayDir.x * t0);
         pIntersect->y = rayOrigin.y + (rayDir.y * t0);
         pIntersect->z = rayOrigin.z + (rayDir.z * t0);
      }
      return true;
   }

   float t1 = (-b + discr) * 0.5f;
   if (t1 > -FLT_EPSILON)
   {
      if (pIntersect != NULL)
      {
         pIntersect->x = rayOrigin.x + (rayDir.x * t1);
         pIntersect->y = rayOrigin.y + (rayDir.y * t1);
         pIntersect->z = rayOrigin.z + (rayDir.z * t1);
      }
      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool RayIntersectPlane(const tVec3 & rayOrigin, const tVec3 & rayDir,
                       const tVec3 & planeNormal, tVec3::value_type planeD,
                       tVec3 * pIntersect)
{
   tVec3::value_type dotProd = planeNormal.Dot(rayDir);

   if (dotProd == 0)
      return false;

   tVec3::value_type t = -(planeNormal.x * rayOrigin.x +
                           planeNormal.y * rayOrigin.y +
                           planeNormal.z * rayOrigin.z + planeD) / dotProd;

   if (t < 0)
      return false;

   if (pIntersect != NULL)
   {
      *pIntersect = rayOrigin + (rayDir * t);
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cRaycastTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cRaycastTests);
      CPPUNIT_TEST(testRayHitSphere);
      CPPUNIT_TEST(testRayHitPlane);
   CPPUNIT_TEST_SUITE_END();

public:
   void testRayHitSphere()
   {
      CPPUNIT_ASSERT(RayIntersectSphere(tVec3(0,0,0),tVec3(0,0,-1),tVec3(0,0,-3),1,NULL));
      CPPUNIT_ASSERT(!RayIntersectSphere(tVec3(0,0,-5),tVec3(0,0,-1),tVec3(0,0,-3),1,NULL));
      CPPUNIT_ASSERT(!RayIntersectSphere(tVec3(0,0,0),tVec3(0,0,1),tVec3(0,0,-3),1,NULL));
   }

   void testRayHitPlane()
   {
      CPPUNIT_ASSERT(RayIntersectPlane(tVec3(0,0,0),tVec3(0,0,-1),tVec3(0,0,-3),-1,NULL));
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(cRaycastTests);

#endif

///////////////////////////////////////////////////////////////////////////////
