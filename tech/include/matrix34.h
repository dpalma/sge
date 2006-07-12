///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX34_H
#define INCLUDED_MATRIX34_H

#include "techdll.h"
#include "matrix3.h"
#include "quat.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef NO_DEFAULT_MATRIX34
template <typename T> class cMatrix34;
typedef cMatrix34<float> tMatrix34;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix34
//
// A 3x3 matrix (rotation) with an extra column (to represent a translation)

template <typename T>
class cMatrix34
{
public:
   cMatrix34();
   cMatrix34(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22, T m03, T m13, T m23);
   cMatrix34(const cMatrix34 & other);

   const cMatrix34 & operator =(const cMatrix34 & other);

   static const cMatrix34 & GetIdentity();

   void SetRotation(const cQuat<T> & q);
   void SetRotation(const cMatrix3<T> & r);
   void SetTranslation(const cVec3<T> & t);

   void Compose(const cMatrix34 & other, cMatrix34 * pResult) const;

   void Transform(const cVec3<T> & v, cVec3<T> * pResult) const;
   void Transform3fv(const float * pV, float * pDest) const;

   // [ m00 m01 m02 m03 ]
   // [ m10 m11 m12 m13 ]
   // [ m20 m21 m22 m23 ]

   union
   {
      struct
      {
         T m00,m10,m20,m01,m11,m21,m02,m12,m22,m03,m13,m23;
      };

      T m[12];
   };
};

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34()
{
}

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34(T m00, T m10, T m20,
                        T m01, T m11, T m21,
                        T m02, T m12, T m22,
                        T m03, T m13, T m23)
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
   this->m03 = m03;
   this->m13 = m13;
   this->m23 = m23;
}

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34(const cMatrix34 & other)
{
   operator =(other);
}

///////////////////////////////////////

template <typename T>
const cMatrix34<T> & cMatrix34<T>::operator =(const cMatrix34 & other)
{
   for (int i = 0; i < _countof(m); i++)
   {
      m[i] = other.m[i];
   }
   return *this;
}

///////////////////////////////////////

template <typename T>
const cMatrix34<T> & cMatrix34<T>::GetIdentity()
{
   static const cMatrix34<T> identity(1,0,0,0,1,0,0,0,1,0,0,0);
   return identity;
}

///////////////////////////////////////

template <typename T>
void cMatrix34<T>::SetRotation(const cQuat<T> & q)
{
   cMatrix3<T> m;
   q.ToMatrix(&m);
   SetRotation(m);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::SetRotation(const cMatrix3<T> & r)
{
   memcpy(&m[0], &r.m[0], 9 * sizeof(T));
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::SetTranslation(const cVec3<T> & t)
{
   memcpy(&m[10], &t.v[0], 3 * sizeof(T));
}

///////////////////////////////////////

template <typename T>
void cMatrix34<T>::Compose(const cMatrix34 & other, cMatrix34 * pResult) const
{
   Assert(pResult != NULL);

#define LHS(row,col)  m[(col<<2)+row]
#define RHS(row,col)  other.m[(col<<2)+row]

   pResult->m[0] = LHS(0, 0) * RHS(0, 0) + LHS(0, 1) * RHS(1, 0) + LHS(0, 2) * RHS(2, 0);
   pResult->m[1] = LHS(1, 0) * RHS(0, 0) + LHS(1, 1) * RHS(1, 0) + LHS(1, 2) * RHS(2, 0);
   pResult->m[2] = LHS(2, 0) * RHS(0, 0) + LHS(2, 1) * RHS(1, 0) + LHS(2, 2) * RHS(2, 0);

   pResult->m[3] = LHS(0, 0) * RHS(0, 1) + LHS(0, 1) * RHS(1, 1) + LHS(0, 2) * RHS(2, 1);
   pResult->m[4] = LHS(1, 0) * RHS(0, 1) + LHS(1, 1) * RHS(1, 1) + LHS(1, 2) * RHS(2, 1);
   pResult->m[5] = LHS(2, 0) * RHS(0, 1) + LHS(2, 1) * RHS(1, 1) + LHS(2, 2) * RHS(2, 1);

   pResult->m[6] = LHS(0, 0) * RHS(0, 2) + LHS(0, 1) * RHS(1, 2) + LHS(0, 2) * RHS(2, 2);
   pResult->m[7] = LHS(1, 0) * RHS(0, 2) + LHS(1, 1) * RHS(1, 2) + LHS(1, 2) * RHS(2, 2);
   pResult->m[8] = LHS(2, 0) * RHS(0, 2) + LHS(2, 1) * RHS(1, 2) + LHS(2, 2) * RHS(2, 2);

#undef LHS
#undef RHS

   pResult->m[9] = m[9] + other.m[9];
   pResult->m[10] = m[10] + other.m[10];
   pResult->m[11] = m[11] + other.m[11];
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::Transform(const cVec3<T> & v, cVec3<T> * pResult) const
{
   Assert(pResult != NULL);
   // [ m00 m01 m02 m03 ]   [v0]
   // [ m10 m11 m12 m13 ] * [v1] = [v0' v1' v2' v3']
   // [ m20 m21 m22 m23 ]   [v2]
   //                       [v3]
   pResult->x = (v.x * m00) + (v.y * m01) + (v.z * m02) + m03;
   pResult->y = (v.x * m10) + (v.y * m11) + (v.z * m12) + m13;
   pResult->z = (v.x * m20) + (v.y * m21) + (v.z * m22) + m23;
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::Transform3fv(const float * pV, float * pDest) const
{
   pDest[0] = (pV[0] * m00) + (pV[1] * m01) + (pV[2] * m02) + m03;
   pDest[1] = (pV[0] * m10) + (pV[1] * m11) + (pV[2] * m12) + m13;
   pDest[2] = (pV[0] * m20) + (pV[1] * m21) + (pV[2] * m22) + m23;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX34_H
