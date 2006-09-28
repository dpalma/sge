///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cColorImpl
//

template <typename T>
class cColorImpl
{
public:
   cColorImpl();
   cColorImpl(T r, T g, T b);
   cColorImpl(T r, T g, T b, T a);
   explicit cColorImpl(const T rgba[4]);
   cColorImpl(const cColorImpl & other);
   const cColorImpl & operator =(const cColorImpl & other);

   bool operator !=(const cColorImpl & other);
   bool operator ==(const cColorImpl & other);

   const cColorImpl<T> & operator *=(const cColorImpl & other);
   const cColorImpl<T> & operator /=(const cColorImpl & other);
   const cColorImpl<T> & operator +=(const cColorImpl & other);
   const cColorImpl<T> & operator -=(const cColorImpl & other);

   T GetRed() const;
   T GetGreen() const;
   T GetBlue() const;
   T GetAlpha() const;

   T GetGrayLevel() const;

   const T * GetPointer() const;

   union
   {
      struct
      {
         T r, g, b, a;
      };
      T rgba[3];
   };
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

class cColor : public cColorImpl<float>
{
public:
   cColor() : cColorImpl<float>() {}
   cColor(float r, float g, float b) : cColorImpl<float>(r,g,b) {}
   cColor(float r, float g, float b, float a) : cColorImpl<float>(r,g,b,a) {}
   explicit cColor(const float rgba[4]) : cColorImpl<float>(rgba) {}
   cColor(const cColor & other) : cColorImpl<float>(other) {}
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
