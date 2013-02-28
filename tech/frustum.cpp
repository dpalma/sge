///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/frustum.h"

#include "tech/axisalignedbox.h"
#include "tech/plane.inl"
#include "tech/vec3.h"

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFrustum
//

///////////////////////////////////////

cFrustum::cFrustum()
{
}

///////////////////////////////////////

cFrustum::cFrustum(const tPlane planes[kMaxFrustumPlanes])
{
   memcpy(m_planes, planes, sizeof(m_planes));
}

///////////////////////////////////////

cFrustum::~cFrustum()
{
}

///////////////////////////////////////

bool cFrustum::PointInFrustum(const tVec3 & point) const
{
   for (int p = 0; p < kMaxFrustumPlanes; p++)
   {
      if (m_planes[p].a * point.x + m_planes[p].b * point.y + m_planes[p].c * point.z + m_planes[p].d <= 0)
         return false;
   }
   return true;
}

///////////////////////////////////////

bool cFrustum::SphereInFrustum(const tVec3 & center, float radius) const
{
   for (int p = 0; p < kMaxFrustumPlanes; p++)
   {
      if (m_planes[p].a * center.x + m_planes[p].b * center.y + m_planes[p].c * center.z + m_planes[p].d <= -radius)
         return false;
   }
   return true;
}

///////////////////////////////////////

bool cFrustum::BoxInFrustum(const tAxisAlignedBox & box) const
{
   const cPoint3<float> & maxs = box.GetMaxs();
   const cPoint3<float> & mins = box.GetMins();
   for (int p = 0; p < kMaxFrustumPlanes; p++)
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
