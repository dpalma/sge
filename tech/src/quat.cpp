/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "quat.h"
#include "vec3.h"
#include "matrix4.h"

#include <cmath>

#include "dbgalloc.h" // must be last header

// REFERENCES
// "Rotating Objects Using Quaternions", February, 1998 "Game Developer", pp. 34-42

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cQuat
//

///////////////////////////////////////

cQuat::cQuat(const tVec3 & v, value_type _w)
 : x(v.x), y(v.y), z(v.z), w(_w)
{
}

///////////////////////////////////////////////////////////////////////////////

void cQuat::ToMatrix(sMatrix4 * pMatrix) const
{
   tQuat::value_type s = 2.0f / Norm();
   tQuat::value_type xs = x*s;
   tQuat::value_type ys = y*s;
   tQuat::value_type zs = z*s;

   tQuat::value_type wx = w*xs;
   tQuat::value_type wy = w*ys;
   tQuat::value_type wz = w*zs;

   tQuat::value_type xx = x*xs;
   tQuat::value_type xy = x*ys;
   tQuat::value_type xz = x*zs;

   tQuat::value_type yy = y*ys;
   tQuat::value_type yz = y*zs;
   tQuat::value_type zz = z*zs;

   pMatrix->m[0] = 1 - (yy + zz);
   pMatrix->m[1] = xy + wz;
   pMatrix->m[2] = xz - wy;
   pMatrix->m[3] = 0;

   pMatrix->m[4] = xy - wz;
   pMatrix->m[5] = 1 - (xx + zz);
   pMatrix->m[6] = yz + wx;
   pMatrix->m[7] = 0;

   pMatrix->m[8] = xz + wy;
   pMatrix->m[9] = yz - wx;
   pMatrix->m[10] = 1 - (xx + yy);
   pMatrix->m[11] = 0;

   pMatrix->m[12] = 0;
   pMatrix->m[13] = 0;
   pMatrix->m[14] = 0;
   pMatrix->m[15] = 1;
}

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
