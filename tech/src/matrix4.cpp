///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "matrix4.h"
// cVec2 is not used but is included here to instantiate the exports
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

#include <cmath>
#include <cfloat>
#include <memory.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

typedef bool (* tMatrixInvertFn)(const float *, float *);
typedef void (* tMatrixMultiplyFn)(const float *, const float *, float *);
typedef bool (* tMatrixOpQuerySupportFn)();

///////////////////////////////////////////////////////////////////////////////

bool MatrixInvertByCramersRule(const float *mat, float *dst)
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

bool MatrixInvertByCramersRuleSupported()
{
   return true;
}

///////////////////////////////////////////////////////////////////////////////

static const struct
{
   tMatrixInvertFn pfnInvert;
   tMatrixOpQuerySupportFn pfnSupport;
}
g_matrixInvertFns[] =
{
   { MatrixInvertByCramersRule, MatrixInvertByCramersRuleSupported },
};

////////////////////////////////////////

bool MatrixInvertInit(const float * m, float * pResult);

tMatrixInvertFn g_pfnMatrixInvert = MatrixInvertInit;

////////////////////////////////////////

bool MatrixInvertInit(const float * m, float * pResult)
{
   for (size_t i = 0; i < _countof(g_matrixInvertFns); i++)
   {
      if (g_matrixInvertFns[i].pfnSupport != NULL && 
         (*g_matrixInvertFns[i].pfnSupport)())
      {
         g_pfnMatrixInvert = g_matrixInvertFns[i].pfnInvert;
         break;
      }
   }
   Assert(g_pfnMatrixInvert != NULL);
   return (*g_pfnMatrixInvert)(m, pResult);
}

////////////////////////////////////////

bool MatrixInvert(const float * m, float * pResult)
{
   Assert(g_pfnMatrixInvert != NULL);
   return (*g_pfnMatrixInvert)(m, pResult);
}

///////////////////////////////////////////////////////////////////////////////
// Default C code for matrix multiplication

void MatrixMultiplyDefault(const float * ml, const float * mr, float * pResult)
{
   Assert(ml != NULL);
   Assert(mr != NULL);
   Assert(pResult != NULL);

#define LHS(row,col)  ml[(col<<2)+row]
#define RHS(row,col)  mr[(col<<2)+row]

   // lhsRow0 * rhsCol0
   // lhsRow1 * rhsCol0
   // lhsRow2 * rhsCol0
   // lhsRow3 * rhsCol0
   pResult[0] = LHS(0, 0) * RHS(0, 0) + LHS(0, 1) * RHS(1, 0) + LHS(0, 2) * RHS(2, 0) + LHS(0, 3) * RHS(3, 0);
   pResult[1] = LHS(1, 0) * RHS(0, 0) + LHS(1, 1) * RHS(1, 0) + LHS(1, 2) * RHS(2, 0) + LHS(1, 3) * RHS(3, 0);
   pResult[2] = LHS(2, 0) * RHS(0, 0) + LHS(2, 1) * RHS(1, 0) + LHS(2, 2) * RHS(2, 0) + LHS(2, 3) * RHS(3, 0);
   pResult[3] = LHS(3, 0) * RHS(0, 0) + LHS(3, 1) * RHS(1, 0) + LHS(3, 2) * RHS(2, 0) + LHS(3, 3) * RHS(3, 0);

   pResult[4] = LHS(0, 0) * RHS(0, 1) + LHS(0, 1) * RHS(1, 1) + LHS(0, 2) * RHS(2, 1) + LHS(0, 3) * RHS(3, 1);
   pResult[5] = LHS(1, 0) * RHS(0, 1) + LHS(1, 1) * RHS(1, 1) + LHS(1, 2) * RHS(2, 1) + LHS(1, 3) * RHS(3, 1);
   pResult[6] = LHS(2, 0) * RHS(0, 1) + LHS(2, 1) * RHS(1, 1) + LHS(2, 2) * RHS(2, 1) + LHS(2, 3) * RHS(3, 1);
   pResult[7] = LHS(3, 0) * RHS(0, 1) + LHS(3, 1) * RHS(1, 1) + LHS(3, 2) * RHS(2, 1) + LHS(3, 3) * RHS(3, 1);

   pResult[8] = LHS(0, 0) * RHS(0, 2) + LHS(0, 1) * RHS(1, 2) + LHS(0, 2) * RHS(2, 2) + LHS(0, 3) * RHS(3, 2);
   pResult[9] = LHS(1, 0) * RHS(0, 2) + LHS(1, 1) * RHS(1, 2) + LHS(1, 2) * RHS(2, 2) + LHS(1, 3) * RHS(3, 2);
   pResult[10] = LHS(2, 0) * RHS(0, 2) + LHS(2, 1) * RHS(1, 2) + LHS(2, 2) * RHS(2, 2) + LHS(2, 3) * RHS(3, 2);
   pResult[11] = LHS(3, 0) * RHS(0, 2) + LHS(3, 1) * RHS(1, 2) + LHS(3, 2) * RHS(2, 2) + LHS(3, 3) * RHS(3, 2);

   pResult[12] = LHS(0, 0) * RHS(0, 3) + LHS(0, 1) * RHS(1, 3) + LHS(0, 2) * RHS(2, 3) + LHS(0, 3) * RHS(3, 3);
   pResult[13] = LHS(1, 0) * RHS(0, 3) + LHS(1, 1) * RHS(1, 3) + LHS(1, 2) * RHS(2, 3) + LHS(1, 3) * RHS(3, 3);
   pResult[14] = LHS(2, 0) * RHS(0, 3) + LHS(2, 1) * RHS(1, 3) + LHS(2, 2) * RHS(2, 3) + LHS(2, 3) * RHS(3, 3);
   pResult[15] = LHS(3, 0) * RHS(0, 3) + LHS(3, 1) * RHS(1, 3) + LHS(3, 2) * RHS(2, 3) + LHS(3, 3) * RHS(3, 3);

#undef LHS
#undef RHS
}

bool MatrixMultiplyDefaultSupported()
{
   return true; // always supported
}

///////////////////////////////////////////////////////////////////////////////

static const struct
{
   tMatrixMultiplyFn pfnMultiply;
   tMatrixOpQuerySupportFn pfnSupport;
}
g_matrixMultiplyFns[] =
{
   { MatrixMultiplyDefault, MatrixMultiplyDefaultSupported },
};

////////////////////////////////////////

void MatrixMultiplyInit(const float * ml, const float * mr, float * pResult);

tMatrixMultiplyFn g_pfnMatrixMultiply = MatrixMultiplyInit;

////////////////////////////////////////

void MatrixMultiplyInit(const float * ml, const float * mr, float * pResult)
{
   for (size_t i = 0; i < _countof(g_matrixMultiplyFns); i++)
   {
      if (g_matrixMultiplyFns[i].pfnSupport != NULL && 
         (*g_matrixMultiplyFns[i].pfnSupport)())
      {
         g_pfnMatrixMultiply = g_matrixMultiplyFns[i].pfnMultiply;
         break;
      }
   }
   Assert(g_pfnMatrixMultiply != NULL);
   (*g_pfnMatrixMultiply)(ml, mr, pResult);
}

////////////////////////////////////////

void MatrixMultiply(const float * ml, const float * mr, float * pResult)
{
   Assert(g_pfnMatrixMultiply != NULL);
   (*g_pfnMatrixMultiply)(ml, mr, pResult);
}

///////////////////////////////////////////////////////////////////////////////

void MatrixTransform3Default(const float * m, const float * v, float * pResult)
{
   Assert(m != NULL);
   Assert(v != NULL);
   Assert(pResult != NULL);
   // result.x = row0 * v
   // result.y = row1 * v
   // result.z = row2 * v
   pResult[0] = (v[0] * m[0]) + (v[1] * m[4]) + (v[2] * m[8]) + m[12];
   pResult[1] = (v[0] * m[1]) + (v[1] * m[5]) + (v[2] * m[9]) + m[13];
   pResult[2] = (v[0] * m[2]) + (v[1] * m[6]) + (v[2] * m[10]) + m[14];
}

bool MatrixTransform3DefaultSupported()
{
   return true; // always supported
}

///////////////////////////////////////////////////////////////////////////////

void MatrixTransform3(const float * m, const float * v, float * pResult)
{
   static const struct
   {
      tMatrixMultiplyFn pfnTransform3;
      tMatrixOpQuerySupportFn pfnSupport;
   }
   matrixTransform3Fns[] =
   {
      { MatrixTransform3Default, MatrixTransform3DefaultSupported },
   };

   static tMatrixMultiplyFn pfnMatrixTransform3 = NULL;

   if (pfnMatrixTransform3 == NULL)
   {
      for (size_t i = 0; i < _countof(matrixTransform3Fns); i++)
      {
         if (matrixTransform3Fns[i].pfnSupport != NULL && 
            (*matrixTransform3Fns[i].pfnSupport)())
         {
            pfnMatrixTransform3 = matrixTransform3Fns[i].pfnTransform3;
            break;
         }
      }
   }

   (*pfnMatrixTransform3)(m, v, pResult);
}


///////////////////////////////////////////////////////////////////////////////

void MatrixTransform4Default(const float * m, const float * v, float * pResult)
{
   Assert(m != NULL);
   Assert(v != NULL);
   Assert(pResult != NULL);
   // result.x = row0 * v
   // result.y = row1 * v
   // result.z = row2 * v
   // result.w = row3 * v
   pResult[0] = (v[0] * m[0]) + (v[1] * m[4]) + (v[2] * m[8]) + (v[3] * m[12]);
   pResult[1] = (v[0] * m[1]) + (v[1] * m[5]) + (v[2] * m[9]) + (v[3] * m[13]);
   pResult[2] = (v[0] * m[2]) + (v[1] * m[6]) + (v[2] * m[10]) + (v[3] * m[14]);
   pResult[3] = (v[0] * m[3]) + (v[1] * m[7]) + (v[2] * m[11]) + (v[3] * m[15]);
}

bool MatrixTransform4DefaultSupported()
{
   return true; // always supported
}

///////////////////////////////////////////////////////////////////////////////

void MatrixTransform4(const float * m, const float * v, float * pResult)
{
   static const struct
   {
      tMatrixMultiplyFn pfnTransform4;
      tMatrixOpQuerySupportFn pfnSupport;
   }
   matrixTransform4Fns[] =
   {
      { MatrixTransform4Default, MatrixTransform4DefaultSupported },
   };

   static tMatrixMultiplyFn pfnMatrixTransform4 = NULL;

   if (pfnMatrixTransform4 == NULL)
   {
      for (size_t i = 0; i < _countof(matrixTransform4Fns); i++)
      {
         if (matrixTransform4Fns[i].pfnSupport != NULL && 
            (*matrixTransform4Fns[i].pfnSupport)())
         {
            pfnMatrixTransform4 = matrixTransform4Fns[i].pfnTransform4;
            break;
         }
      }
   }

   (*pfnMatrixTransform4)(m, v, pResult);
}


///////////////////////////////////////////////////////////////////////////////

void MatrixTranslate(float x, float y, float z, tMatrix4 * pResult)
{
   Assert(pResult != NULL);
   memcpy(pResult->m, tMatrix4::GetIdentity().m, sizeof(pResult->m));
   pResult->m03 = x;
   pResult->m13 = y;
   pResult->m23 = z;
}

///////////////////////////////////////////////////////////////////////////////
// See http://www.makegames.com/3drotation

void MatrixRotateX(float theta, tMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   memcpy(pResult->m, tMatrix4::GetIdentity().m, sizeof(pResult->m));
   pResult->m11 = costheta;
   pResult->m12 = -sintheta;
   pResult->m21 = sintheta;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateY(float theta, tMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   memcpy(pResult->m, tMatrix4::GetIdentity().m, sizeof(pResult->m));
   pResult->m00 = costheta;
   pResult->m02 = sintheta;
   pResult->m20 = -sintheta;
   pResult->m22 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateZ(float theta, tMatrix4 * pResult)
{
   float sintheta = sinf(Deg2Rad(theta));
   float costheta = cosf(Deg2Rad(theta));
   Assert(pResult != NULL);
   memcpy(pResult->m, tMatrix4::GetIdentity().m, sizeof(pResult->m));
   pResult->m00 = costheta;
   pResult->m01 = -sintheta;
   pResult->m10 = sintheta;
   pResult->m11 = costheta;
}

///////////////////////////////////////////////////////////////////////////////

// Use the Gram-Schmidt method to determine an orthogonal vector 
// to the plane defined by v1 and v2.
static tVec3 GetOrthogonalVector(const tVec3 & v1, const tVec3 & v2)
{
   tVec3 v1Prime(v1);
   v1Prime.Normalize();

   tVec3::value_type v2dotv1p = v2.Dot(v1Prime);

   return tVec3(v2 - (v1Prime * v2dotv1p));
}

void MatrixLookAt(const tVec3 & eye, const tVec3 & center, const tVec3 & up,
                  tMatrix4 * pMatrix)
{
   tVec3 newZAxis(eye - center);
   newZAxis.Normalize();

   tVec3 lineOfSight(center - eye);
   tVec3 newYAxis = GetOrthogonalVector(lineOfSight, up);
   newYAxis.Normalize();

   tVec3 newXAxis = newYAxis.Cross(newZAxis);
   newXAxis.Normalize();

   Assert(pMatrix != NULL);
   pMatrix->m00 = newXAxis.x;
   pMatrix->m10 = newYAxis.x;
   pMatrix->m20 = newZAxis.x;
   pMatrix->m30 = 0;
   pMatrix->m01 = newXAxis.y;
   pMatrix->m11 = newYAxis.y;
   pMatrix->m21 = newZAxis.y;
   pMatrix->m31 = 0;
   pMatrix->m02 = newXAxis.z;
   pMatrix->m12 = newYAxis.z;
   pMatrix->m22 = newZAxis.z;
   pMatrix->m32 = 0;
   pMatrix->m03 = 0;
   pMatrix->m13 = 0;
   pMatrix->m23 = 0;
   pMatrix->m33 = 1;

   tMatrix4 eyeTrans;
   memcpy(eyeTrans.m, tMatrix4::GetIdentity().m, sizeof(eyeTrans.m));
   eyeTrans.m03 = -eye.x;
   eyeTrans.m13 = -eye.y;
   eyeTrans.m23 = -eye.z;

   tMatrix4 temp;
   pMatrix->Multiply(eyeTrans, &temp);
   *pMatrix = temp;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixPerspective(float fov, float aspect, float znear, float zfar, tMatrix4 * pResult)
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

void MatrixOrtho(float left, float right, float bottom, float top, float znear, float zfar, tMatrix4 * pResult)
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

CPPUNIT_TEST_SUITE_REGISTRATION(cMatrix4Tests);

void cMatrix4Tests::TestMatrixInvert()
{
   tMatrix4 M;
   M.m00 = -.519f; M.m01 = 0;      M.m02 = .854f;  M.m03 = 0;
   M.m10 = -.207f; M.m11 = 0.97f;  M.m12 = -.126f; M.m13 = -1e-08f;
   M.m20 = -.828f; M.m21 = -.242f; M.m22 = -.504f; M.m23 = 4.12f;
   M.m30 = 0;      M.m31 = 0;      M.m32 = 0;      M.m33 = 1;

   tMatrix4 I;
   if (!MatrixInvert(M.m, I.m))
   {
      DebugMsg("Matrix is NOT invertible\n");
      return;
   }

   tMatrix4 result;
   M.Multiply(I, &result);

   CPPUNIT_ASSERT(MatrixIsIdentity(result));
}

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
