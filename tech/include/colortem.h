///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#error ("colortem.h must be preceded by color.h")
#endif

#ifndef INCLUDED_COLORTEM_H
#define INCLUDED_COLORTEM_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cColorImpl
//

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
   Assert(!"Unimplemented instantiation of cColorImpl<>::GetGrayLevel");
}

////////////////////////////////////////

template <>
inline byte cColorImpl<byte>::GetGrayLevel() const
{
   return ((3 * GetRed()) + (6 * GetGreen()) + GetBlue()) / 10;
}

////////////////////////////////////////

template <>
inline double cColorImpl<double>::GetGrayLevel() const
{
   return (0.3 * GetRed()) + (0.6 * GetGreen()) + (0.1 * GetBlue());
}

////////////////////////////////////////

template <>
inline float cColorImpl<float>::GetGrayLevel() const
{
   return (0.3f * GetRed()) + (0.6f * GetGreen()) + (0.1f * GetBlue());
}

////////////////////////////////////////

template <typename T>
inline const T * cColorImpl<T>::GetPointer() const
{
   return rgba;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLORTEM_H
