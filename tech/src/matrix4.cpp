///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "matrix4.h"
#include "vec3.h"
#include "vec4.h"
#include "techmath.h"
#include <cmath>
#include <cfloat>
#include <memory.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

bool MatrixInvertByCramersRule(float *mat, float *dst)
{
   float tmp[12]; /* temp array for pairs */
   float src[16]; /* array of transpose source matrix */
   float det; /* determinant */
   /* transpose matrix */
   for ( int i = 0; i < 4; i++) {
      src[i] = mat[i*4];
      src[i + 4] = mat[i*4 + 1];
      src[i + 8] = mat[i*4 + 2];
      src[i + 12] = mat[i*4 + 3];
   }
   /* calculate pairs for first 8 elements (cofactors) */
   tmp[0] = src[10] * src[15];
   tmp[1] = src[11] * src[14];
   tmp[2] = src[9] * src[15];
   tmp[3] = src[11] * src[13];
   tmp[4] = src[9] * src[14];
   tmp[5] = src[10] * src[13];
   tmp[6] = src[8] * src[15];
   tmp[7] = src[11] * src[12];
   tmp[8] = src[8] * src[14];
   tmp[9] = src[10] * src[12];
   tmp[10] = src[8] * src[13];
   tmp[11] = src[9] * src[12];
   /* calculate first 8 elements (cofactors) */
   dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
   dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
   dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
   dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
   dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
   dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
   dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
   dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
   dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
   dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
   dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
   dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
   dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
   dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
   dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
   dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
   /* calculate pairs for second 8 elements (cofactors) */
   tmp[0] = src[2]*src[7];
   tmp[1] = src[3]*src[6];
   tmp[2] = src[1]*src[7];
   tmp[3] = src[3]*src[5];
   tmp[4] = src[1]*src[6];
   tmp[5] = src[2]*src[5];
   tmp[6] = src[0]*src[7];
   tmp[7] = src[3]*src[4];
   tmp[8] = src[0]*src[6];
   tmp[9] = src[2]*src[4];
   tmp[10] = src[0]*src[5];
   tmp[11] = src[1]*src[4];
   /* calculate second 8 elements (cofactors) */
   dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
   dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
   dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
   dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
   dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
   dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
   dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
   dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
   dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
   dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
   dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
   dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
   dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
   dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
   dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
   dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
   /* calculate determinant */
   det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
   if (det == 0.0f)
      return false;
   /* calculate matrix inverse */
   det = 1/det;
   for ( int j = 0; j < 16; j++)
      dst[j] *= det;
   return true;
}

///////////////////////////////////////////////////////////////////////////////

sMatrix4 * MatrixInvert(const sMatrix4 & m, sMatrix4 * pInverse)
{
   Assert(pInverse != NULL);
   return MatrixInvertByCramersRule(const_cast<float *>(m.m), pInverse->m) ? pInverse : NULL;
}

///////////////////////////////////////////////////////////////////////////////
// @TODO (dpalma 9-12-02)

sMatrix4 * MatrixInvertByGaussJordan(const sMatrix4 & m, sMatrix4 * pInverse)
{
   Assert(pInverse != NULL);

   tMatrix4 a(m), b;
   b.Identity();

   float p = a.m00;
   a.m00 /= p;
   a.m01 /= p;
   a.m02 /= p;
   a.m03 /= p;

   p = a.m10 / a.m00;
   a.m10 -= a.m00 * p;
   a.m11 -= a.m01 * p;
   a.m12 -= a.m02 * p;
   a.m13 -= a.m03 * p;

   p = a.m20 / a.m00;
   a.m20 -= a.m00 * p;
   a.m21 -= a.m01 * p;
   a.m22 -= a.m02 * p;
   a.m23 -= a.m03 * p;

   p = a.m30 / a.m00;
   a.m30 -= a.m00 * p;
   a.m31 -= a.m01 * p;
   a.m32 -= a.m02 * p;
   a.m33 -= a.m03 * p;

   *pInverse = b;

   return pInverse;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixTranslate(float x, float y, float z, sMatrix4 * pResult)
{
   Assert(pResult != NULL);
   pResult->Identity();
   pResult->m03 = x;
   pResult->m13 = y;
   pResult->m23 = z;
}

///////////////////////////////////////////////////////////////////////////////
// See http://www.makegames.com/3drotation

void MatrixRotateX(float theta, sMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->Identity();
   pResult->m11 = costheta;
   pResult->m12 = -sintheta;
   pResult->m21 = sintheta;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateY(float theta, sMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->Identity();
   pResult->m00 = costheta;
   pResult->m02 = sintheta;
   pResult->m20 = -sintheta;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateZ(float theta, sMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   pResult->Identity();
   pResult->m00 = costheta;
   pResult->m01 = -sintheta;
   pResult->m10 = sintheta;
   pResult->m11 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixPerspective(float fov, float aspect, float znear, float zfar, sMatrix4 * pResult)
{
   static const float kPiOver360 = kPi / 360.0f;

   // fov is specified in degrees; if radians then tan(fov/2) would do
   float top = tanf(fov * kPiOver360) * znear;
   float bottom = -top;

   float left = aspect * bottom;
   float right = aspect * top;

   float a = (right + left) / (right - left);
   float b = (top + bottom) / (top - bottom);
   float c = -(zfar + znear) / (zfar - znear);
   float d = (-2.0f * zfar * znear) / (zfar - znear);

   Assert(pResult != NULL);
   memset(pResult, 0, sizeof(*pResult));
   pResult->m[0] = 2.0f * znear / (right - left);
   pResult->m[5] = 2.0f * znear / (top - bottom);
   pResult->m[8] = a;
   pResult->m[9] = b;
   pResult->m[10] = c;
   pResult->m[11] = -1.0f;
   pResult->m[14] = d;
}

void MatrixPerspective(double fov, double aspect, double znear, double zfar, cMatrix4<double> * pResult)
{
   static const double kPiOver360 = (double)kPi / 360.0;

   // fov is specified in degrees; if radians then tan(fov/2) would do
   double top = tan(fov * kPiOver360) * znear;
   double bottom = -top;

   double left = aspect * bottom;
   double right = aspect * top;

   double a = (right + left) / (right - left);
   double b = (top + bottom) / (top - bottom);
   double c = -(zfar + znear) / (zfar - znear);
   double d = (-2.0f * zfar * znear) / (zfar - znear);

   Assert(pResult != NULL);
   memset(pResult, 0, sizeof(*pResult));
   pResult->m[0] = 2.0f * znear / (right - left);
   pResult->m[5] = 2.0f * znear / (top - bottom);
   pResult->m[8] = a;
   pResult->m[9] = b;
   pResult->m[10] = c;
   pResult->m[11] = -1.0f;
   pResult->m[14] = d;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixOrtho(float left, float right, float bottom, float top, float znear, float zfar, sMatrix4 * pResult)
{
   // formulas obtained from documentation of glOrtho
   Assert(pResult != NULL);
   memset(pResult, 0, sizeof(*pResult));
   pResult->m[0] = 2.0f  / (right - left);
   pResult->m[5] = 2.0f  / (top - bottom);
   pResult->m[10] = -2.0f / (zfar - znear);
   pResult->m[12] = -(right + left) / (right - left);
   pResult->m[13] = -(top + bottom) / (top - bottom);
   pResult->m[14] = -(zfar + znear) / (zfar - znear);
   pResult->m[15] = 1;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

static bool MatrixIsIdentity(const tMatrix4 & m)
{
   static const float kFloatEpsilon = 2 * FLT_EPSILON;

   for (int i = 0; i < 4; i++)
   {
      for (int j = 0; j < 4; j++)
      {
         if (i == j)
         {
            //if (m.m[j * 4 + i] != 1.0f)
            if (fabs(m.m[j * 4 + i] - 1) > kFloatEpsilon)
               return false;
         }
         else
         {
            //if (m.m[j * 4 + i] != 0.0f)
            if (fabs(m.m[j * 4 + i]) > kFloatEpsilon)
               return false;
         }
      }
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////

class cMatrix4Tests : public CppUnit::TestCase
{
   void TestMatrixInvert();

   CPPUNIT_TEST_SUITE(cMatrix4Tests);
      CPPUNIT_TEST(TestMatrixInvert);
   CPPUNIT_TEST_SUITE_END();
};

void cMatrix4Tests::TestMatrixInvert()
{
   sMatrix4 M;
   M.m00 = -.519f; M.m01 = 0;      M.m02 = .854f;  M.m03 = 0;
   M.m10 = -.207f; M.m11 = 0.97f;  M.m12 = -.126f; M.m13 = -1e-08f;
   M.m20 = -.828f; M.m21 = -.242f; M.m22 = -.504f; M.m23 = 4.12f;
   M.m30 = 0;      M.m31 = 0;      M.m32 = 0;      M.m33 = 1;

   sMatrix4 I;
   if (!MatrixInvert(M, &I))
   {
      DebugMsg("Matrix is NOT invertible\n");
      return;
   }

   sMatrix4 result = M * I;

   CPPUNIT_ASSERT(MatrixIsIdentity(result));
}

CPPUNIT_TEST_SUITE_REGISTRATION(cMatrix4Tests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
