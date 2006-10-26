///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "tech/color.h"

#include "tech/techmath.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

////////////////////////////////////////

cColor::cColor()
{
}

////////////////////////////////////////

cColor::cColor(value_type r, value_type g, value_type b)
{
   rgba[0] = r;
   rgba[1] = g;
   rgba[2] = b;
   rgba[3] = 1;
}

////////////////////////////////////////

cColor::cColor(value_type r, value_type g, value_type b, value_type a)
{
   rgba[0] = r;
   rgba[1] = g;
   rgba[2] = b;
   rgba[3] = a;
}

////////////////////////////////////////

cColor::cColor(const value_type rgba[4])
{
   r = rgba[0];
   g = rgba[1];
   b = rgba[2];
   a = rgba[3];
}

////////////////////////////////////////

cColor::cColor(const cColor & other)
{
   rgba[0] = other.rgba[0];
   rgba[1] = other.rgba[1];
   rgba[2] = other.rgba[2];
   rgba[3] = other.rgba[3];
}

////////////////////////////////////////

const cColor & cColor::operator =(const cColor & other)
{
   rgba[0] = other.rgba[0];
   rgba[1] = other.rgba[1];
   rgba[2] = other.rgba[2];
   rgba[3] = other.rgba[3];
   return *this;
}

////////////////////////////////////////

bool cColor::operator !=(const cColor & other)
{
   return
      !AlmostEqual(rgba[0], other.rgba[0]) ||
      !AlmostEqual(rgba[1], other.rgba[1]) ||
      !AlmostEqual(rgba[2], other.rgba[2]) ||
      !AlmostEqual(rgba[3], other.rgba[3]);
}

////////////////////////////////////////

bool cColor::operator ==(const cColor & other)
{
   return
      AlmostEqual(rgba[0], other.rgba[0]) &&
      AlmostEqual(rgba[1], other.rgba[1]) &&
      AlmostEqual(rgba[2], other.rgba[2]) &&
      AlmostEqual(rgba[3], other.rgba[3]);
}

////////////////////////////////////////

const cColor & cColor::operator *=(const cColor & other)
{
   rgba[0] *= other.rgba[0];
   rgba[1] *= other.rgba[1];
   rgba[2] *= other.rgba[2];
   rgba[3] *= other.rgba[3];
   return *this;
}

////////////////////////////////////////

const cColor & cColor::operator /=(const cColor & other)
{
   rgba[0] /= other.rgba[0];
   rgba[1] /= other.rgba[1];
   rgba[2] /= other.rgba[2];
   rgba[3] /= other.rgba[3];
   return *this;
}

////////////////////////////////////////

const cColor & cColor::operator +=(const cColor & other)
{
   rgba[0] += other.rgba[0];
   rgba[1] += other.rgba[1];
   rgba[2] += other.rgba[2];
   rgba[3] += other.rgba[3];
   return *this;
}

////////////////////////////////////////

const cColor & cColor::operator -=(const cColor & other)
{
   rgba[0] -= other.rgba[0];
   rgba[1] -= other.rgba[1];
   rgba[2] -= other.rgba[2];
   rgba[3] -= other.rgba[3];
   return *this;
}

////////////////////////////////////////

bool cColor::GetHSV(float hsv[3]) const
{
   if (hsv == NULL)
   {
      return false;
   }
   value_type mn = Min(r, Min(g, b));
   value_type mx = Max(r, Max(g, b));
   value_type h = -1, s = 0, v = mx;
   if (mx != 0)
   {
      value_type delta = mx - mn;
      s = delta / mx;
      if (r == mx)
      {
         // between yellow & magenta
         h = (g - b) / delta;
      }
      else if (g == mx)
      {
         // between cyan & yellow
         h = 2 + (b - r) / delta;
      }
      else
      {
         // between magenta & cyan
         h = 4 + (r - g) / delta;
      }
      h *= 60;
      if (h < 0)
      {
         h += 360;
      }
   }
   hsv[0] = h;
   hsv[1] = s;
   hsv[2] = v;
   return true;
}

////////////////////////////////////////

bool cColor::SetHSV(const float hsv[3])
{
   if (hsv == NULL)
   {
      return false;
   }
   if (hsv[1] == 0)
   {
      // achromatic (grey)
      r = g = b = hsv[2];
      return true;
   }
   value_type h = hsv[0] / 60; // sector 0 to 5
   int i = FloatToInt(floor(h));
   value_type f = h - i; // factorial part of h
   value_type p = hsv[2] * (1 - hsv[1]);
   value_type q = hsv[2] * (1 - hsv[1] * f);
   value_type t = hsv[2] * (1 - hsv[1] * (1 - f));
   switch (i)
   {
      case 0:
         r = hsv[2];
         g = t;
         b = p;
         break;
      case 1:
         r = q;
         g = hsv[2];
         b = p;
         break;
      case 2:
         r = p;
         g = hsv[2];
         b = t;
         break;
      case 3:
         r = p;
         g = q;
         b = hsv[2];
         break;
      case 4:
         r = t;
         g = p;
         b = hsv[2];
         break;
      default:	// case 5:
         r = hsv[2];
         g = p;
         b = q;
         break;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////
