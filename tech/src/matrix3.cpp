///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "matrix3.h"
#include "vec3.h"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include <cmath>
#include <cfloat>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
// See http://www.makegames.com/3drotation

void MatrixRotateX(float theta, tMatrix3 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->m00 = 1;
   pResult->m10 = 0;
   pResult->m20 = 0;
   pResult->m01 = 0;
   pResult->m11 = costheta;
   pResult->m21 = sintheta;
   pResult->m02 = 0;
   pResult->m12 = -sintheta;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateY(float theta, tMatrix3 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->m00 = costheta;
   pResult->m10 = 0;
   pResult->m20 = -sintheta;
   pResult->m01 = 0;
   pResult->m11 = 1;
   pResult->m21 = 0;
   pResult->m02 = sintheta;
   pResult->m12 = 0;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateZ(float theta, tMatrix3 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->m00 = costheta;
   pResult->m10 = sintheta;
   pResult->m20 = 0;
   pResult->m01 = -sintheta;
   pResult->m11 = costheta;
   pResult->m21 = 0;
   pResult->m02 = 0;
   pResult->m12 = 0;
   pResult->m22 = 1;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

static bool MatrixIsIdentity(const tMatrix3 & m)
{
   static const float kFloatEpsilon = 2 * FLT_EPSILON;

   for (int i = 0; i < 3; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         if (i == j)
         {
            if (fabs(m.m[j * 3 + i] - 1) > kFloatEpsilon)
               return false;
         }
         else
         {
            if (fabs(m.m[j * 3 + i]) > kFloatEpsilon)
               return false;
         }
      }
   }
   return true;
}

TEST(MatrixRotation)
{
   // Simple rotation about x-axis
   {
      tMatrix3 mr;
      MatrixRotateX(90, &mr);
      tVec3 p(0, 1, 0), pp;
      mr.Transform(p, &pp);
      CHECK(fabs(pp.x - 0.0f) < 0.00001);
      CHECK(fabs(pp.y - 0.0f) < 0.00001);
      CHECK(fabs(pp.z - 1.0f) < 0.00001);
   }

   // Composite rotation
   {
      // TODO
   }
}

#endif // HAVE_UNITTESTPP

///////////////////////////////////////////////////////////////////////////////
