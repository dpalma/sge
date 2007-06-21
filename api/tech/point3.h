///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_POINT3_H
#define INCLUDED_POINT3_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPoint3
//

template <typename T>
class cPoint3
{
public:
   typedef T value_type;
   typedef const cPoint3<T> & const_reference;

   cPoint3();
   cPoint3(value_type xx, value_type yy, value_type zz);
   cPoint3(const value_type xyz[3]);
   cPoint3(const_reference other);

   const_reference operator =(const_reference other);
   const_reference operator +=(const_reference other);
   const_reference operator -=(const_reference other);
   const_reference operator *=(value_type scale);
   const_reference operator /=(value_type divisor);

   union
   {
      struct
      {
         value_type x, y, z;
      };
      value_type xyz[3];
   };
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_POINT3_H

