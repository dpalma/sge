///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec3;
template <typename T> class cVec4;

#ifndef NO_DEFAULT_MATRIX4
template <typename T> class cMatrix4;
typedef cMatrix4<float> tMatrix4;
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class cMatrix4
{
public:
   cMatrix4();
   cMatrix4(const cMatrix4 & other);

   const cMatrix4 & operator =(const cMatrix4 & other);

   void Identity();
   void Transpose();

   template <typename VT>
   cVec3<VT> Transform(const cVec3<VT> & v) const
   {
      return cVec3<VT>(
         v.x*m00 + v.y*m01 + v.z*m02 + m03,
         v.x*m10 + v.y*m11 + v.z*m12 + m13,
         v.x*m20 + v.y*m21 + v.z*m22 + m23);
   }

   template <typename VT>
   cVec4<VT> Transform(const cVec4<VT> & v) const
   {
      return cVec4<VT>(
         v.x*m00 + v.y*m01 + v.z*m02 + v.w*m03,
         v.x*m10 + v.y*m11 + v.z*m12 + v.w*m13,
         v.x*m20 + v.y*m21 + v.z*m22 + v.w*m23,
         v.x*m30 + v.y*m31 + v.z*m32 + v.w*m33);
   }

   union
   {
      struct
      {
         T m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33;
      };

      T m[16];
   };
};

///////////////////////////////////////

template <typename T>
cMatrix4<T>::cMatrix4()
{
}

///////////////////////////////////////

template <typename T>
cMatrix4<T>::cMatrix4(const cMatrix4 & other)
{
   for (int i = 0; i < _countof(m); i++)
      m[i] = other.m[i];
}

///////////////////////////////////////

template <typename T>
const cMatrix4<T> & cMatrix4<T>::operator =(const cMatrix4 & other)
{
   for (int i = 0; i < _countof(m); i++)
      m[i] = other.m[i];
   return *this;
}

///////////////////////////////////////

template <typename T>
void cMatrix4<T>::Identity()
{
   m00 = 1; m01 = 0; m02 = 0; m03 = 0;
   m10 = 0; m11 = 1; m12 = 0; m13 = 0;
   m20 = 0; m21 = 0; m22 = 1; m23 = 0;
   m30 = 0; m31 = 0; m32 = 0; m33 = 1;
}

///////////////////////////////////////

template <typename T>
void cMatrix4<T>::Transpose()
{
   Swap(m10, m01);
   Swap(m12, m21);
   Swap(m13, m31);
   Swap(m20, m02);
   Swap(m21, m12);
   Swap(m23, m32);
   Swap(m30, m03);
   Swap(m31, m13);
   Swap(m32, m23);
}

///////////////////////////////////////

template <typename T>
cMatrix4<T> operator *(const cMatrix4<T> & a, const cMatrix4<T> & b)
{
   cMatrix4<T> result;

#define A(row,col)  a.m[(col<<2)+row]
#define B(row,col)  b.m[(col<<2)+row]
#define T(row,col)  result.m[(col<<2)+row]

   for (int i = 0; i < 4; i++)
   {
      T(i, 0) = A(i, 0) * B(0, 0) + A(i, 1) * B(1, 0) + A(i, 2) * B(2, 0) + A(i, 3) * B(3, 0);
      T(i, 1) = A(i, 0) * B(0, 1) + A(i, 1) * B(1, 1) + A(i, 2) * B(2, 1) + A(i, 3) * B(3, 1);
      T(i, 2) = A(i, 0) * B(0, 2) + A(i, 1) * B(1, 2) + A(i, 2) * B(2, 2) + A(i, 3) * B(3, 2);
      T(i, 3) = A(i, 0) * B(0, 3) + A(i, 1) * B(1, 3) + A(i, 2) * B(2, 3) + A(i, 3) * B(3, 3);
   }

#undef A
#undef B
#undef T

   return result;
}

///////////////////////////////////////////////////////////////////////////////
// matrix operations

// @TODO: "sMatrix4" is the old name. Everybody should move to "tMatrix4".
typedef cMatrix4<float> sMatrix4;

TECH_API sMatrix4 * MatrixInvert(const sMatrix4 & m, sMatrix4 * pResult);

///////////////////////////////////////////////////////////////////////////////
// 3D transformations

TECH_API void MatrixTranslate(float x, float y, float z, sMatrix4 * pResult);
TECH_API void MatrixRotateX(float theta, sMatrix4 * pResult);
TECH_API void MatrixRotateY(float theta, sMatrix4 * pResult);
TECH_API void MatrixRotateZ(float theta, sMatrix4 * pResult);
TECH_API void MatrixPerspective(float fov, float aspect, float znear, float zfar, sMatrix4 * pResult);
TECH_API void MatrixPerspective(double fov, double aspect, double znear, double zfar, cMatrix4<double> * pResult);
TECH_API void MatrixOrtho(float left, float right, float bottom, float top, float znear, float zfar, sMatrix4 * pResult);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX4_H
