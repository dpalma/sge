///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#define ARGB(a,r,g,b) \
   (((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define RGBA(r,g,b,a) \
   ARGB(r,g,b,a)

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

   uint32 ToARGB8888() const;

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

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl()
{
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(T r, T g, T b)
{
   rgba[0] = r;
   rgba[1] = g;
   rgba[2] = b;
   rgba[3] = 1;
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(T r, T g, T b, T a)
{
   rgba[0] = r;
   rgba[1] = g;
   rgba[2] = b;
   rgba[3] = a;
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(const T rgba[4])
{
   r = rgba[0];
   g = rgba[1];
   b = rgba[2];
   a = rgba[3];
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(const cColorImpl & other)
{
   rgba[0] = other.rgba[0];
   rgba[1] = other.rgba[1];
   rgba[2] = other.rgba[2];
   rgba[3] = other.rgba[3];
}

////////////////////////////////////////

template <typename T>
inline const cColorImpl<T> & cColorImpl<T>::operator =(const cColorImpl & other)
{
   rgba[0] = other.rgba[0];
   rgba[1] = other.rgba[1];
   rgba[2] = other.rgba[2];
   rgba[3] = other.rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
inline bool cColorImpl<T>::operator !=(const cColorImpl & other)
{
   return
      !AlmostEqual(rgba[0], other.rgba[0]) ||
      !AlmostEqual(rgba[1], other.rgba[1]) ||
      !AlmostEqual(rgba[2], other.rgba[2]) ||
      !AlmostEqual(rgba[3], other.rgba[3]);
}

////////////////////////////////////////

template <typename T>
inline bool cColorImpl<T>::operator ==(const cColorImpl & other)
{
   return
      AlmostEqual(rgba[0], other.rgba[0]) &&
      AlmostEqual(rgba[1], other.rgba[1]) &&
      AlmostEqual(rgba[2], other.rgba[2]) &&
      AlmostEqual(rgba[3], other.rgba[3]);
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator *=(const cColorImpl & other)
{
   rgba[0] *= other.rgba[0];
   rgba[1] *= other.rgba[1];
   rgba[2] *= other.rgba[2];
   rgba[3] *= other.rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator /=(const cColorImpl & other)
{
   rgba[0] /= other.rgba[0];
   rgba[1] /= other.rgba[1];
   rgba[2] /= other.rgba[2];
   rgba[3] /= other.rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator +=(const cColorImpl & other)
{
   rgba[0] += other.rgba[0];
   rgba[1] += other.rgba[1];
   rgba[2] += other.rgba[2];
   rgba[3] += other.rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator -=(const cColorImpl & other)
{
   rgba[0] -= other.rgba[0];
   rgba[1] -= other.rgba[1];
   rgba[2] -= other.rgba[2];
   rgba[3] -= other.rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetRed() const
{
   return rgba[0];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetGreen() const
{
   return rgba[1];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetBlue() const
{
   return rgba[2];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetAlpha() const
{
   return rgba[3];
}

////////////////////////////////////////

template <typename T>
T cColorImpl<T>::GetGrayLevel() const
{
   return (0.3 * GetRed()) + (0.6 * GetGreen()) + (0.1 * GetBlue());
}

////////////////////////////////////////

template <typename T>
inline uint32 cColorImpl<T>::ToARGB8888() const
{
   Assert(!"Do not use the default version of ToARGB8888");
   return ~0;
}

////////////////////////////////////////

template <>
inline uint32 cColorImpl<float>::ToARGB8888() const
{
   return ARGB(
      static_cast<byte>(GetAlpha() * 255.0f),
      static_cast<byte>(GetRed() * 255.0f),
      static_cast<byte>(GetGreen() * 255.0f),
      static_cast<byte>(GetBlue() * 255.0f));
}

////////////////////////////////////////

template <>
inline uint32 cColorImpl<double>::ToARGB8888() const
{
   return ARGB(
      static_cast<byte>(GetAlpha() * 255.0),
      static_cast<byte>(GetRed() * 255.0),
      static_cast<byte>(GetGreen() * 255.0),
      static_cast<byte>(GetBlue() * 255.0));
}

////////////////////////////////////////

template <>
inline uint32 cColorImpl<byte>::ToARGB8888() const
{
   return ARGB(GetAlpha(), GetRed(), GetGreen(), GetBlue());
}

////////////////////////////////////////

template <typename T>
inline const T * cColorImpl<T>::GetPointer() const
{
   return rgba;
}


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
