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

   // [ m00 m01 m02 ]   [ n00 n01 n02 ]   [ m00n00+m01n10+m02n20 m00n01+m01n11+m02n21 m00n02+m01n12+m02n22 ]
   // [ m10 m11 m12 ] * [ n10 n11 n12 ] = [ m10n00+m11n10+m12n20 m10n01+m11n11+m12n21 m10n02+m11n12+m12n22 ]
   // [ m20 m21 m22 ]   [ n20 n21 n22 ]   [ m20n00+m21n10+m22n20 m20n01+m21n11+m22n21 m20n02+m21n12+m22n22 ]

   pResult->m00 = m00*other.m00 + m01*other.m10 + m02*other.m20;
   pResult->m10 = m10*other.m00 + m11*other.m10 + m12*other.m20;
   pResult->m20 = m20*other.m00 + m21*other.m10 + m22*other.m20;
   pResult->m01 = m00*other.m01 + m01*other.m11 + m02*other.m21;
   pResult->m11 = m10*other.m01 + m11*other.m11 + m12*other.m21;
   pResult->m21 = m20*other.m01 + m21*other.m11 + m22*other.m21;
   pResult->m02 = m00*other.m02 + m01*other.m12 + m02*other.m22;
   pResult->m12 = m10*other.m02 + m11*other.m12 + m12*other.m22;
   pResult->m22 = m20*other.m02 + m21*other.m12 + m22*other.m22;
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
