///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#include "techdll.h"

#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

class TECH_API cColor
{
public:
   typedef float value_type;

   cColor();
   cColor(value_type r, value_type g, value_type b);
   cColor(value_type r, value_type g, value_type b, value_type a);
   explicit cColor(const value_type rgb[4]);
   cColor(const cColor & other);
   const cColor & operator =(const cColor & other);

   bool operator !=(const cColor & other);
   bool operator ==(const cColor & other);

   const cColor & operator *=(const cColor & other);
   const cColor & operator /=(const cColor & other);
   const cColor & operator +=(const cColor & other);
   const cColor & operator -=(const cColor & other);

   inline const value_type * GetPointer() const { return rgba; }

   union
   {
      struct
      {
         value_type r, g, b, a;
      };
      value_type rgba[4];
   };
};

TECH_API tResult RGBToHSV(const float rgb[3], float hsv[3]);

TECH_API tResult HSVToRGB(const float hsv[3], float rgb[3]);


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
