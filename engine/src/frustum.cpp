///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "frustum.h"

#include "vec3.h"
#include "matrix4.h"

#include <cmath>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPlane
//

void cPlane::Normalize()
{
   float t = 1.0f / sqrtf(a * a + b * b + c * c);
   a *= t;
   b *= t;
   c *= t;
   d *= t;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFrustum
//

///////////////////////////////////////

void cFrustum::ExtractPlanes(const sMatrix4 & modelviewProjection)
{
   // Determine the right plane
   m_planes[0].a = modelviewProjection.m[3] - modelviewProjection.m[0];
   m_planes[0].b = modelviewProjection.m[7] - modelviewProjection.m[4];
   m_planes[0].c = modelviewProjection.m[11] - modelviewProjection.m[8];
   m_planes[0].d = modelviewProjection.m[15] - modelviewProjection.m[12];
   m_planes[0].Normalize();

   // Determine the left plane
   m_planes[1].a = modelviewProjection.m[3] + modelviewProjection.m[0];
   m_planes[1].b = modelviewProjection.m[7] + modelviewProjection.m[4];
   m_planes[1].c = modelviewProjection.m[11] + modelviewProjection.m[8];
   m_planes[1].d = modelviewProjection.m[15] + modelviewProjection.m[12];
   m_planes[1].Normalize();

   // Determine the bottom plane
   m_planes[2].a = modelviewProjection.m[3] + modelviewProjection.m[1];
   m_planes[2].b = modelviewProjection.m[7] + modelviewProjection.m[5];
   m_planes[2].c = modelviewProjection.m[11] + modelviewProjection.m[9];
   m_planes[2].d = modelviewProjection.m[15] + modelviewProjection.m[13];
   m_planes[2].Normalize();

   // Determine the top plane
   m_planes[3].a = modelviewProjection.m[3] - modelviewProjection.m[1];
   m_planes[3].b = modelviewProjection.m[7] - modelviewProjection.m[5];
   m_planes[3].c = modelviewProjection.m[11] - modelviewProjection.m[9];
   m_planes[3].d = modelviewProjection.m[15] - modelviewProjection.m[13];
   m_planes[3].Normalize();

   // Determine the far plane
   m_planes[4].a = modelviewProjection.m[3] - modelviewProjection.m[2];
   m_planes[4].b = modelviewProjection.m[7] - modelviewProjection.m[6];
   m_planes[4].c = modelviewProjection.m[11] - modelviewProjection.m[10];
   m_planes[4].d = modelviewProjection.m[15] - modelviewProjection.m[14];
   m_planes[4].Normalize();

   // Determine the near plane
   m_planes[5].a = modelviewProjection.m[3] + modelviewProjection.m[2];
   m_planes[5].b = modelviewProjection.m[7] + modelviewProjection.m[6];
   m_planes[5].c = modelviewProjection.m[11] + modelviewProjection.m[10];
   m_planes[5].d = modelviewProjection.m[15] + modelviewProjection.m[14];
   m_planes[5].Normalize();
}

///////////////////////////////////////

bool cFrustum::PointInFrustum(const tVec3 & point) const
{
   for (int p = 0; p < 6; p++)
   {
      if (m_planes[p].a * point.x + m_planes[p].b * point.y + m_planes[p].c * point.z + m_planes[p].d <= 0)
         return false;
   }
   return true;
}

///////////////////////////////////////

bool cFrustum::SphereInFrustum(const tVec3 & center, float radius) const
{
   for (int p = 0; p < 6; p++)
   {
      if (m_planes[p].a * center.x + m_planes[p].b * center.y + m_planes[p].c * center.z + m_planes[p].d <= -radius)
         return false;
   }
   return true;
}

///////////////////////////////////////

bool cFrustum::BoxInFrustum(const tVec3 & maxs, const tVec3 & mins) const
{
   for (int p = 0; p < 6; p++)
   {
      if (m_planes[p].a * mins.x + m_planes[p].b * mins.y + m_planes[p].c * mins.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * maxs.x + m_planes[p].b * mins.y + m_planes[p].c * mins.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * mins.x + m_planes[p].b * maxs.y + m_planes[p].c * mins.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * maxs.x + m_planes[p].b * maxs.y + m_planes[p].c * mins.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * mins.x + m_planes[p].b * mins.y + m_planes[p].c * maxs.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * maxs.x + m_planes[p].b * mins.y + m_planes[p].c * maxs.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * mins.x + m_planes[p].b * maxs.y + m_planes[p].c * maxs.z + m_planes[p].d > 0)
         continue;
      if (m_planes[p].a * maxs.x + m_planes[p].b * maxs.y + m_planes[p].c * maxs.z + m_planes[p].d > 0)
         continue;
      return false;
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////////
