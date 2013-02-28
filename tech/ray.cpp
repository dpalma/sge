////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/ray.h"
#include "tech/ray.inl"

#include "tech/point3.inl"
#include "tech/techmath.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cstdlib>

#include "tech/dbgalloc.h" // must be last header

////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

////////////////////////////////////////

TEST(RayIntersectsSphere)
{
   CHECK(cRay<float>(cPoint3<float>(0,0,0),tVec3(0,0,-1)).IntersectsSphere(cPoint3<float>(0,0,-3),1));
   CHECK(!cRay<float>(cPoint3<float>(0,0,-5),tVec3(0,0,-1)).IntersectsSphere(cPoint3<float>(0,0,-3),1));
   CHECK(!cRay<float>(cPoint3<float>(0,0,0),tVec3(0,0,1)).IntersectsSphere(cPoint3<float>(0,0,-3),1));
}

////////////////////////////////////////

TEST(RayIntersectsPlane)
{
   CHECK(cRay<float>(cPoint3<float>(0,0,0),tVec3(0,0,-1)).IntersectsPlane(cVec3<float>(0,0,-3),-1));
}

////////////////////////////////////////

TEST(RayIntersectsAxisAlignedBox)
{
   float maxDim = -FLT_MAX;
   cPoint3<float> min(static_cast<float>(rand()), static_cast<float>(rand()), static_cast<float>(rand()));
   cPoint3<float> max(static_cast<float>(rand()), static_cast<float>(rand()), static_cast<float>(rand()));
   for (int i = 0; i < 3; i++)
   {
      if (min.xyz[i] > max.xyz[i])
      {
         float temp = min.xyz[i];
         min.xyz[i] = max.xyz[i];
         max.xyz[i] = temp;
      }
      if ((max.xyz[i] - min.xyz[i]) > maxDim)
      {
         maxDim = max.xyz[i] - min.xyz[i];
      }
   }
   tAxisAlignedBox box(min, max);
   cPoint3<float> centroid;
   box.GetCentroid(&centroid);
   // determine a point way out there in space for the ray origin
   tVec3 dir(static_cast<float>(rand()), static_cast<float>(rand()), static_cast<float>(rand()));
   dir.Normalize();
   cPoint3<float> origin(
      centroid.x + (dir.x * (maxDim * 2)),
      centroid.y + (dir.y * (maxDim * 2)),
      centroid.z + (dir.z * (maxDim * 2)));
   // point the ray back toward the center of the box
   dir.x = -dir.x;
   dir.y = -dir.y;
   dir.z = -dir.z;
   cRay<float> ray(origin, dir);
   CHECK(ray.IntersectsAxisAlignedBox(box));
}

#endif // HAVE_UNITTESTPP

////////////////////////////////////////////////////////////////////////////////
