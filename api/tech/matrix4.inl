///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX4_INL
#define INCLUDED_MATRIX4_INL

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix4
//

///////////////////////////////////////

template <typename T>
cMatrix4<T>::cMatrix4()
{
}

///////////////////////////////////////

template <typename T>
cMatrix4<T>::cMatrix4(T m[16])
{
   memcpy(this->m, m, sizeof(T) * 16);
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
cMatrix4<T> cMatrix4<T>::GetTranspose() const
{
   return cMatrix4<T>(m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33);
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
cPoint3<T> cMatrix4<T>::Transform(const cPoint3<T> & p) const
{
   return cPoint3<T>(
      (p.x * m[0]) + (p.y * m[4]) + (p.z * m[8]) + m[12],
      (p.x * m[1]) + (p.y * m[5]) + (p.z * m[9]) + m[13],
      (p.x * m[2]) + (p.y * m[6]) + (p.z * m[10]) + m[14]);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> cMatrix4<T>::Transform(const cVec3<T> & v) const
{
   // result.x = row0 * v
   // result.y = row1 * v
   // result.z = row2 * v
   return cVec3<T>(
      (v.x * m[0]) + (v.y * m[4]) + (v.z * m[8]),
      (v.x * m[1]) + (v.y * m[5]) + (v.z * m[9]),
      (v.x * m[2]) + (v.y * m[6]) + (v.z * m[10]));
}

///////////////////////////////////////

template <typename T>
inline cVec4<T> cMatrix4<T>::Transform(const cVec4<T> & v) const
{
   // result.x = row0 * v
   // result.y = row1 * v
   // result.z = row2 * v
   // result.w = row3 * v
   return cVec4<T>(
      (v.x * m[0]) + (v.y * m[4]) + (v.z * m[8]) + (v.w * m[12]),
      (v.x * m[1]) + (v.y * m[5]) + (v.z * m[9]) + (v.w * m[13]),
      (v.x * m[2]) + (v.y * m[6]) + (v.z * m[10]) + (v.w * m[14]),
      (v.x * m[3]) + (v.y * m[7]) + (v.z * m[11]) + (v.w * m[15]));
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX4_INL
