///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ray.h"

#include <cfloat>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRay
//

///////////////////////////////////////

cRay::cRay()
{
}

///////////////////////////////////////

cRay::cRay(const tVec3 & origin, const tVec3 & direction)
 : m_origin(origin),
   m_direction(direction)
{
}

///////////////////////////////////////

cRay::cRay(const cRay & ray)
 : m_origin(ray.m_origin),
   m_direction(ray.m_direction)
{
}

///////////////////////////////////////

const cRay & cRay::operator =(const cRay & ray)
{
   m_origin = ray.m_origin;
   m_direction = ray.m_direction;
   return *this;
}

///////////////////////////////////////

bool cRay::IntersectsSphere(const tVec3 & center, 
                            tVec3::value_type radius, 
                            tVec3 * pIntersection /*=NULL*/) const
{
   // Don't need to calculate the 'a' term because the ray is a unit vector

   // Calculate the 'b' term 
   float b = 2 *((GetDirection().x * (GetOrigin().x - center.x)) +
                 (GetDirection().y * (GetOrigin().y - center.y)) +
                 (GetDirection().z * (GetOrigin().z - center.z)));

   // Calculate the 'c' term 
   float c = sqr(GetOrigin().x - center.x) +
             sqr(GetOrigin().y - center.y) +
             sqr(GetOrigin().z - center.z) -
             sqr(radius);

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
      if (pIntersection != NULL)
      {
         *pIntersection = GetOrigin() + (GetDirection() * t0);
      }
      return true;
   }

   float t1 = (-b + discr) * 0.5f;
   if (t1 > -FLT_EPSILON)
   {
      if (pIntersection != NULL)
      {
         *pIntersection = GetOrigin() + (GetDirection() * t1);
      }
      return true;
   }

   return false;
}

///////////////////////////////////////

bool cRay::IntersectsPlane(const tVec3 & normal, 
                           tVec3::value_type d,
                           tVec3 * pIntersection /*=NULL*/) const
{
   tVec3::value_type dotProd = normal.Dot(GetDirection());

   if (dotProd == 0)
      return false;

   tVec3::value_type t = -(normal.x * GetOrigin().x +
                           normal.y * GetOrigin().y +
                           normal.z * GetOrigin().z + d) / dotProd;

   if (t < 0)
      return false;

   if (pIntersection != NULL)
   {
      *pIntersection = GetOrigin() + (GetDirection() * t);
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cRayTests : public CppUnit::TestCase
{
   void TestIntersectsSphere();
   void TestIntersectsPlane();

   CPPUNIT_TEST_SUITE(cRayTests);
      CPPUNIT_TEST(TestIntersectsSphere);
      CPPUNIT_TEST(TestIntersectsPlane);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cRayTests);

void cRayTests::TestIntersectsSphere()
{
   CPPUNIT_ASSERT(cRay(tVec3(0,0,0),tVec3(0,0,-1)).IntersectsSphere(tVec3(0,0,-3),1));
   CPPUNIT_ASSERT(!cRay(tVec3(0,0,-5),tVec3(0,0,-1)).IntersectsSphere(tVec3(0,0,-3),1));
   CPPUNIT_ASSERT(!cRay(tVec3(0,0,0),tVec3(0,0,1)).IntersectsSphere(tVec3(0,0,-3),1));
}

void cRayTests::TestIntersectsPlane()
{
   CPPUNIT_ASSERT(cRay(tVec3(0,0,0),tVec3(0,0,-1)).IntersectsPlane(tVec3(0,0,-3),-1));
}

#endif

///////////////////////////////////////////////////////////////////////////////
