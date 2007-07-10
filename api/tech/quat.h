/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_QUAT_H
#define INCLUDED_QUAT_H

#include "matrix3.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

template <typename T> class cVec3;

#ifndef NO_DEFAULT_QUAT
template <typename T> class cQuat;
typedef cQuat<float> tQuat;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cQuat
//

template <typename T>
class cQuat
{
public:
   typedef T value_type;

   cQuat();
   cQuat(const cVec3<T> & axis, T angle); // angle should be in radians
   cQuat(value_type xx, value_type yy, value_type zz, value_type ww);
   cQuat(const cQuat & other);

   const cQuat & operator =(const cQuat & other);

   void Assign(const cQuat & other);

   static const cQuat & GetMultIdentity();

   bool EqualTo(const cQuat & other) const;

   const cQuat<T> & operator *=(const cQuat & other);

   value_type Dot(const cQuat & quat) const;
   value_type Norm() const;
   cQuat<T> Inverse() const;

   cMatrix3<T> ToMatrix() const;

   static cQuat<T> FromEulerAngles(T pitch, T yaw, T roll);

   union
   {
      struct
      {
         value_type x, y, z, w;
      };
      value_type q[4];
   };
};

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_QUAT_H
