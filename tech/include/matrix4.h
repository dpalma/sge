///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H

#include "techdll.h"

#ifdef _MSC_VER
#pragma once
#endif

template <typename T> class cVec3;
typedef cVec3<float> tVec3;
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

   void Transform(const float * pSource, float * pDest) const;

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
   operator =(other);
}

///////////////////////////////////////

template <typename T>
const cMatrix4<T> & cMatrix4<T>::operator =(const cMatrix4 & other)
{
   for (int i = 0; i < _countof(m); i++)
   {
      m[i] = other.m[i];
   }
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
   T temp;
   temp = m10, m10 = m01, m01 = temp;
   temp = m12, m12 = m21, m21 = temp;
   temp = m13, m13 = m31, m31 = temp;
   temp = m20, m20 = m02, m02 = temp;
   temp = m21, m21 = m12, m12 = temp;
   temp = m23, m23 = m32, m32 = temp;
   temp = m30, m30 = m03, m03 = temp;
   temp = m31, m31 = m13, m13 = temp;
   temp = m32, m32 = m23, m23 = temp;
}

///////////////////////////////////////

template <typename T>
inline void cMatrix4<T>::Transform(const float * pSource, float * pDest) const
{
   pDest[0] = (pSource[0] * m00) + (pSource[1] * m01) + (pSource[2] * m02) + m03;
   pDest[1] = (pSource[0] * m10) + (pSource[1] * m11) + (pSource[2] * m12) + m13;
   pDest[2] = (pSource[0] * m20) + (pSource[1] * m21) + (pSource[2] * m22) + m23;
}

///////////////////////////////////////

template <typename T>
inline cMatrix4<T> operator *(const cMatrix4<T> & a, const cMatrix4<T> & b)
{
   cMatrix4<T> result;
   MatrixMultiply(a.m, b.m, result.m);
   return result;
}

///////////////////////////////////////////////////////////////////////////////
// matrix operations

TECH_API bool MatrixInvert(const float * m, float * pResult);
TECH_API void MatrixMultiply(const float * ml, const float * mr, float * pResult);

///////////////////////////////////////////////////////////////////////////////
// 3D transformations

TECH_API void MatrixTranslate(float x, float y, float z, tMatrix4 * pResult);
TECH_API void MatrixRotateX(float theta, tMatrix4 * pResult);
TECH_API void MatrixRotateY(float theta, tMatrix4 * pResult);
TECH_API void MatrixRotateZ(float theta, tMatrix4 * pResult);
TECH_API void MatrixLookAt(const tVec3 & eye, const tVec3 & center, const tVec3 & up,
                           tMatrix4 * pMatrix);
TECH_API void MatrixPerspective(float fov, float aspect, float znear, float zfar, tMatrix4 * pResult);
TECH_API void MatrixPerspective(double fov, double aspect, double znear, double zfar, cMatrix4<double> * pResult);
TECH_API void MatrixOrtho(float left, float right, float bottom, float top, float znear, float zfar, tMatrix4 * pResult);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX4_H
