///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX4_H
#define INCLUDED_MATRIX4_H

#include "techdll.h"
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
   cMatrix4();
   cMatrix4(T m00, T m10, T m20, T m30, T m01, T m11, T m21, T m31,
            T m02, T m12, T m22, T m32, T m03, T m13, T m23, T m33);
   cMatrix4(const cMatrix4 & other);

   const cMatrix4 & operator =(const cMatrix4 & other);

   static const cMatrix4 & GetIdentity();

   void Transpose(cMatrix4 * pDest);

   void Multiply(const cMatrix4 & other, cMatrix4 * pResult) const;

   void Transform(const cVec3<T> & v, cVec3<T> * pResult) const;
   void Transform(const cVec4<T> & v, cVec4<T> * pResult) const;

   void Transform3(const float * pV, float * pDest) const;
   void Transform4(const float * pV, float * pDest) const;

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
cMatrix4<T>::cMatrix4(T m00, T m10, T m20, T m30, T m01, T m11, T m21, T m31,
                      T m02, T m12, T m22, T m32, T m03, T m13, T m23, T m33)
{
   this->m00 = m00;
   this->m10 = m10;
   this->m20 = m20;
   this->m30 = m30;
   this->m01 = m01;
   this->m11 = m11;
   this->m21 = m21;
   this->m31 = m31;
   this->m02 = m02;
   this->m12 = m12;
   this->m22 = m22;
   this->m32 = m32;
   this->m03 = m03;
   this->m13 = m13;
   this->m23 = m23;
   this->m33 = m33;
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
const cMatrix4<T> & cMatrix4<T>::GetIdentity()
{
   static const cMatrix4<T> identity(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
   return identity;
}

///////////////////////////////////////

template <typename T>
void cMatrix4<T>::Transpose(cMatrix4<T> * pDest)
{
   Assert(pDest != NULL);
   pDest->m00 = m00;
   pDest->m10 = m01;
   pDest->m20 = m02;
   pDest->m30 = m03;
   pDest->m01 = m10;
   pDest->m11 = m11;
   pDest->m21 = m12;
   pDest->m31 = m13;
   pDest->m02 = m20;
   pDest->m12 = m21;
   pDest->m22 = m22;
   pDest->m32 = m23;
   pDest->m03 = m30;
   pDest->m13 = m31;
   pDest->m23 = m32;
   pDest->m33 = m33;
}

///////////////////////////////////////

template <typename T>
void cMatrix4<T>::Multiply(const cMatrix4 & other, cMatrix4 * pResult) const
{
   Assert(pResult != NULL);
   MatrixMultiply(m, other.m, pResult->m);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix4<T>::Transform(const cVec3<T> & v, cVec3<T> * pResult) const
{
   Assert(pResult != NULL);
   MatrixTransform3(m, v.v, pResult->v);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix4<T>::Transform(const cVec4<T> & v, cVec4<T> * pResult) const
{
   Assert(pResult != NULL);
   MatrixTransform4(m, v.v, pResult->v);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix4<T>::Transform3(const float * pV, float * pDest) const
{
   MatrixTransform3(m, pV, pDest);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix4<T>::Transform4(const float * pV, float * pDest) const
{
   MatrixTransform4(m, pV, pDest);
}

///////////////////////////////////////////////////////////////////////////////
// matrix operations

bool MatrixInvert(const float * m, float * pResult);
void MatrixMultiply(const float * ml, const float * mr, float * pResult);
void MatrixTransform3(const float * m, const float * v, float * pResult);
void MatrixTransform4(const float * m, const float * v, float * pResult);

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
