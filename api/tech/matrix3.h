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

   cMatrix3<T> GetTranspose() const;

   void Multiply(const cMatrix3 & other, cMatrix3 * pResult) const;

   cVec3<T> Transform(const cVec3<T> & v) const;

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

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX3_H
