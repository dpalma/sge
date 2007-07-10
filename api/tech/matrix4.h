///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H

#include "techdll.h"
#include "point3.h"
#include "vec3.h"
#include "vec4.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef NO_DEFAULT_MATRIX4
template <typename T> class cMatrix4;
typedef cMatrix4<float> tMatrix4;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix4
//

template <typename T>
class cMatrix4
{
public:
   typedef T value_type;

   cMatrix4();
   cMatrix4(T m[16]);
   cMatrix4(T m00, T m10, T m20, T m30, T m01, T m11, T m21, T m31,
            T m02, T m12, T m22, T m32, T m03, T m13, T m23, T m33);
   cMatrix4(const cMatrix4 & other);

   const cMatrix4<T> & operator =(const cMatrix4<T> & other);

   static const cMatrix4<T> & GetIdentity();

   cMatrix4<T> GetTranspose() const;

   void Multiply(const cMatrix4 & other, cMatrix4 * pResult) const;

   cPoint3<T> Transform(const cPoint3<T> & p) const;
   cVec3<T> Transform(const cVec3<T> & v) const;
   cVec4<T> Transform(const cVec4<T> & v) const;

   union
   {
      struct
      {
         T m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33;
      };

      T m[16];
   };
};

///////////////////////////////////////////////////////////////////////////////
// matrix operations

bool MatrixInvert(const float * m, float * pResult);
void MatrixMultiply(const float * ml, const float * mr, float * pResult);

///////////////////////////////////////////////////////////////////////////////
// 3D transformations

void MatrixTranslate(float x, float y, float z, tMatrix4 * pResult);
void MatrixRotateX(float theta, tMatrix4 * pResult);
void MatrixRotateY(float theta, tMatrix4 * pResult);
void MatrixRotateZ(float theta, tMatrix4 * pResult);
void MatrixLookAt(const tVec3 & eye, const tVec3 & center, const tVec3 & up, tMatrix4 * pMatrix);
void MatrixPerspective(float fov, float aspect, float znear, float zfar, tMatrix4 * pResult);
void MatrixPerspective(double fov, double aspect, double znear, double zfar, cMatrix4<double> * pResult);
void MatrixOrtho(float left, float right, float bottom, float top, float znear, float zfar, tMatrix4 * pResult);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX4_H
