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

///////////////////////////////////////////////////////////////////////////////

tResult RGBToHSV(const float rgb[3], float hsv[3])
{
   if (rgb == NULL || hsv == NULL)
   {
      return E_POINTER;
   }
   float mn = Min(rgb[0], Min(rgb[1], rgb[2]));
   float mx = Max(rgb[0], Max(rgb[1], rgb[2]));
   float h = -1, s = 0, v = mx;
   if (mx != 0)
   {
      float delta = mx - mn;
      s = delta / mx;
      if (rgb[0] == mx)
      {
         // between yellow & magenta
         h = (rgb[1] - rgb[2]) / delta;
      }
      else if (rgb[1] == mx)
      {
         // between cyan & yellow
         h = 2 + (rgb[2] - rgb[0]) / delta;
      }
      else
      {
         // between magenta & cyan
         h = 4 + (rgb[0] - rgb[1]) / delta;
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
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult HSVToRGB(const float hsv[3], float rgb[3])
{
   if (hsv == NULL || rgb == NULL)
   {
      return E_POINTER;
   }
   if (hsv[1] == 0)
   {
      // achromatic (grey)
      rgb[0] = rgb[1] = rgb[2] = hsv[2];
      return true;
   }
   float h = hsv[0] / 60; // sector 0 to 5
   int i = FloatToInt(floor(h));
   float f = h - i; // factorial part of h
   float p = hsv[2] * (1 - hsv[1]);
   float q = hsv[2] * (1 - hsv[1] * f);
   float t = hsv[2] * (1 - hsv[1] * (1 - f));
   switch (i)
   {
      case 0:
         rgb[0] = hsv[2];
         rgb[1] = t;
         rgb[2] = p;
         break;
      case 1:
         rgb[0] = q;
         rgb[1] = hsv[2];
         rgb[2] = p;
         break;
      case 2:
         rgb[0] = p;
         rgb[1] = hsv[2];
         rgb[2] = t;
         break;
      case 3:
         rgb[0] = p;
         rgb[1] = q;
         rgb[2] = hsv[2];
         break;
      case 4:
         rgb[0] = t;
         rgb[1] = p;
         rgb[2] = hsv[2];
         break;
      default:	// case 5:
         rgb[0] = hsv[2];
         rgb[1] = p;
         rgb[2] = q;
         break;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////
