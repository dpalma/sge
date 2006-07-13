///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX3_H
#define INCLUDED_MATRIX3_H

#include "techdll.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef NO_DEFAULT_MATRIX3
template <typename T> class cMatrix3;
typedef cMatrix3<float> tMatrix3;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix3
//

template <typename T>
class cMatrix3
{
public:
   typedef T value_type;

   cMatrix3();
   cMatrix3(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22);
   cMatrix3(const cMatrix3 & other);

   const cMatrix3 & operator =(const cMatrix3 & other);

   static const cMatrix3 & GetIdentity();

   T GetDeterminant() const;

   void GetTranspose(cMatrix3 * pDest);

   void Multiply(const cMatrix3 & other, cMatrix3 * pResult) const;

   void Transform(const cVec3<T> & v, cVec3<T> * pResult) const;

   void Transform3(const float * pV, float * pDest) const;

   // [ m00 m01 m02 ]
   // [ m10 m11 m12 ]
   // [ m20 m21 m22 ]

   union
   {
      struct
      {
         T m00,m10,m20,m01,m11,m21,m02,m12,m22;
      };

      T m[9];
   };
};

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3()
{
}

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22)
{
   this->m00 = m00;
   this->m10 = m10;
   this->m20 = m20;
   this->m01 = m01;
   this->m11 = m11;
   this->m21 = m21;
   this->m02 = m02;
   this->m12 = m12;
   this->m22 = m22;
}

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3(const cMatrix3 & other)
{
   operator =(other);
}

///////////////////////////////////////

template <typename T>
const cMatrix3<T> & cMatrix3<T>::operator =(const cMatrix3 & other)
{
   for (int i = 0; i < _countof(m); i++)
   {
      m[i] = other.m[i];
   }
   return *this;
}

///////////////////////////////////////

template <typename T>
const cMatrix3<T> & cMatrix3<T>::GetIdentity()
{
   static const cMatrix3<T> identity(1,0,0,0,1,0,0,0,1);
   return identity;
}

///////////////////////////////////////

template <typename T>
T cMatrix3<T>::GetDeterminant() const
{
   return m00 * ((m11 * m22) - (m12 * m21))
      - m01 * ((m10 * m22) - (m12 * m20))
      + m02 * ((m10 * m21) - (m11 * m20));
}

///////////////////////////////////////

template <typename T>
void cMatrix3<T>::GetTranspose(cMatrix3<T> * pDest)
{
   Assert(pDest != NULL);
   pDest->m00 = m00;
   pDest->m10 = m01;
   pDest->m20 = m02;
   pDest->m01 = m10;
   pDest->m11 = m11;
   pDest->m21 = m12;
   pDest->m02 = m20;
   pDest->m12 = m21;
   pDest->m22 = m22;
}

///////////////////////////////////////

template <typename T>
void cMatrix3<T>::Multiply(const cMatrix3 & other, cMatrix3 * pResult) const
{
   Assert(pResult != NULL);

#define LHS(row,col)  ml[(col<<2)+row]
#define RHS(row,col)  mr[(col<<2)+row]

   pResult[0] = LHS(0, 0) * RHS(0, 0) + LHS(0, 1) * RHS(1, 0) + LHS(0, 2) * RHS(2, 0);
   pResult[1] = LHS(1, 0) * RHS(0, 0) + LHS(1, 1) * RHS(1, 0) + LHS(1, 2) * RHS(2, 0);
   pResult[2] = LHS(2, 0) * RHS(0, 0) + LHS(2, 1) * RHS(1, 0) + LHS(2, 2) * RHS(2, 0);

   pResult[3] = LHS(0, 0) * RHS(0, 1) + LHS(0, 1) * RHS(1, 1) + LHS(0, 2) * RHS(2, 1);
   pResult[4] = LHS(1, 0) * RHS(0, 1) + LHS(1, 1) * RHS(1, 1) + LHS(1, 2) * RHS(2, 1);
   pResult[5] = LHS(2, 0) * RHS(0, 1) + LHS(2, 1) * RHS(1, 1) + LHS(2, 2) * RHS(2, 1);

   pResult[6] = LHS(0, 0) * RHS(0, 2) + LHS(0, 1) * RHS(1, 2) + LHS(0, 2) * RHS(2, 2);
   pResult[7] = LHS(1, 0) * RHS(0, 2) + LHS(1, 1) * RHS(1, 2) + LHS(1, 2) * RHS(2, 2);
   pResult[8] = LHS(2, 0) * RHS(0, 2) + LHS(2, 1) * RHS(1, 2) + LHS(2, 2) * RHS(2, 2);

#undef LHS
#undef RHS
}

///////////////////////////////////////

template <typename T>
inline void cMatrix3<T>::Transform(const cVec3<T> & v, cVec3<T> * pResult) const
{
   Assert(pResult != NULL);
   // [ m00 m01 m02 ]   [vx]
   // [ m10 m11 m12 ] * [vy] = [vx' vy' vz']
   // [ m20 m21 m22 ]   [vz]
   pResult->x = (v.x * m00) + (v.y * m01) + (v.z * m02);
   pResult->y = (v.x * m10) + (v.y * m11) + (v.z * m12);
   pResult->z = (v.x * m20) + (v.y * m21) + (v.z * m22);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix3<T>::Transform3(const float * pV, float * pDest) const
{
   pDest[0] = (pV[0] * m00) + (pV[1] * m01) + (pV[2] * m02);
   pDest[1] = (pV[0] * m10) + (pV[1] * m11) + (pV[2] * m12);
   pDest[2] = (pV[0] * m20) + (pV[1] * m21) + (pV[2] * m22);
}

///////////////////////////////////////////////////////////////////////////////

void MatrixRotateX(float theta, tMatrix3 * pResult);
void MatrixRotateY(float theta, tMatrix3 * pResult);
void MatrixRotateZ(float theta, tMatrix3 * pResult);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX3_H
