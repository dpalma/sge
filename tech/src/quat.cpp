/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "quat.h"

#include <cmath>

#include "dbgalloc.h" // must be last header

// REFERENCES
// "Rotating Objects Using Quaternions", February, 1998 "Game Developer", pp. 34-42

///////////////////////////////////////////////////////////////////////////////

tQuat operator *(const tQuat & q0, const tQuat & q1)
{
   tVec3 vq0(q0.x, q0.y, q0.z);

   tVec3 vq1(q1.x, q1.y, q1.z);

   return tQuat(((vq1 * q0.w) + (vq0 * q1.w)) + vq0.Cross(vq1), q0.w * q1.w - vq0.Dot(vq1));
}

///////////////////////////////////////////////////////////////////////////////
// Construct quaternions for the rotations about each axis, then compose them.

tQuat QuatFromEulerAngles(const tVec3 & eulerAngles)
{
   tQuat::value_type angle = eulerAngles.z * 0.5f;
   tQuat::value_type sy = sinf(angle);
   tQuat::value_type cy = cosf(angle);

   angle = eulerAngles.y * 0.5f;
   tQuat::value_type sp = sinf(angle);
   tQuat::value_type cp = cosf(angle);

   angle = eulerAngles.x * 0.5f;
   tQuat::value_type sr = sinf(angle);
   tQuat::value_type cr = cosf(angle);

   tQuat::value_type crcp = cr * cp;
   tQuat::value_type srsp = sr * sp;

   return tQuat(
      sr * cp * cy - cr * sp * sy,
      cr * sp * cy + sr * cp * sy,
      crcp * sy - srsp * cy,
      crcp * cy + srsp * sy); 
}

///////////////////////////////////////////////////////////////////////////////

tQuat QuatSlerp(const tQuat & q0, const tQuat & q1, tQuat::value_type u)
{
   Assert(u >= 0 && u <= 1);

   static const tQuat::value_type kThreshold = 0.001f;

   tQuat::value_type cosTheta = q0.Dot(q1);

   tQuat q1prime;

   if (cosTheta < 0)
   {
      cosTheta = -cosTheta;
      q1prime = q1.Inverse();
   }
   else
   {
      q1prime = q1;
   }

   tQuat::value_type scale0, scale1;

   if (1 - cosTheta > kThreshold)
   {
      tQuat::value_type theta = acosf(cosTheta);

      tQuat::value_type oneOverSinTheta = 1.0f / sqrtf(1 - (cosTheta * cosTheta));

      scale0 = sinf((1.0f - u) * theta) * oneOverSinTheta;
      scale1 = sinf(u * theta) * oneOverSinTheta;
   }
   else
   {
      scale0 = (1.0f - u);
      scale1 = u;
   }

   return (q0 * scale0) + (q1prime * scale1);
}

/////////////////////////////////////////////////////////////////////////////
