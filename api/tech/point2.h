///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_POINT2_H
#define INCLUDED_POINT2_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cPoint2
//

template <typename T>
class cPoint2
{
public:
   typedef T value_type;

   cPoint2();
   cPoint2(value_type xx, value_type yy);
   cPoint2(const cPoint2 & other);

   const cPoint2 & operator =(const cPoint2 & other);
   const cPoint2 & operator +=(const cPoint2 & other);
   const cPoint2 & operator -=(const cPoint2 & other);
   const cPoint2 & operator *=(value_type scale);
   const cPoint2 & operator /=(value_type divisor);

   union
   {
      struct
      {
         value_type x, y;
      };
      value_type xy[2];
   };
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_POINT2_H
