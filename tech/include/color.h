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
   cColorImpl(float r, float g, float b);
   cColorImpl(float r, float g, float b, float a);
   cColorImpl(const T rgba[4]);
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

private:
   T m_rgba[4];
};

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl()
{
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(float r, float g, float b)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = 1;
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(float r, float g, float b, float a)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = a;
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(const T rgba[4])
{
   m_rgba[0] = rgba[0];
   m_rgba[1] = rgba[1];
   m_rgba[2] = rgba[2];
   m_rgba[3] = rgba[3];
}

////////////////////////////////////////

template <typename T>
inline cColorImpl<T>::cColorImpl(const cColorImpl & other)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
}

////////////////////////////////////////

template <typename T>
inline const cColorImpl<T> & cColorImpl<T>::operator =(const cColorImpl & other)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
inline bool cColorImpl<T>::operator !=(const cColorImpl & other)
{
   return
      !AlmostEqual(m_rgba[0], other.m_rgba[0]) ||
      !AlmostEqual(m_rgba[1], other.m_rgba[1]) ||
      !AlmostEqual(m_rgba[2], other.m_rgba[2]) ||
      !AlmostEqual(m_rgba[3], other.m_rgba[3]);
}

////////////////////////////////////////

template <typename T>
inline bool cColorImpl<T>::operator ==(const cColorImpl & other)
{
   return
      AlmostEqual(m_rgba[0], other.m_rgba[0]) &&
      AlmostEqual(m_rgba[1], other.m_rgba[1]) &&
      AlmostEqual(m_rgba[2], other.m_rgba[2]) &&
      AlmostEqual(m_rgba[3], other.m_rgba[3]);
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator *=(const cColorImpl & other)
{
   m_rgba[0] *= other.m_rgba[0];
   m_rgba[1] *= other.m_rgba[1];
   m_rgba[2] *= other.m_rgba[2];
   m_rgba[3] *= other.m_rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator /=(const cColorImpl & other)
{
   m_rgba[0] /= other.m_rgba[0];
   m_rgba[1] /= other.m_rgba[1];
   m_rgba[2] /= other.m_rgba[2];
   m_rgba[3] /= other.m_rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator +=(const cColorImpl & other)
{
   m_rgba[0] += other.m_rgba[0];
   m_rgba[1] += other.m_rgba[1];
   m_rgba[2] += other.m_rgba[2];
   m_rgba[3] += other.m_rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
const cColorImpl<T> & cColorImpl<T>::operator -=(const cColorImpl & other)
{
   m_rgba[0] -= other.m_rgba[0];
   m_rgba[1] -= other.m_rgba[1];
   m_rgba[2] -= other.m_rgba[2];
   m_rgba[3] -= other.m_rgba[3];
   return *this;
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetRed() const
{
   return m_rgba[0];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetGreen() const
{
   return m_rgba[1];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetBlue() const
{
   return m_rgba[2];
}

////////////////////////////////////////

template <typename T>
inline T cColorImpl<T>::GetAlpha() const
{
   return m_rgba[3];
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
   return ARGB(
      static_cast<byte>(GetAlpha() * 255),
      static_cast<byte>(GetRed() * 255),
      static_cast<byte>(GetGreen() * 255),
      static_cast<byte>(GetBlue() * 255));
}

////////////////////////////////////////

template <typename T>
inline const T * cColorImpl<T>::GetPointer() const
{
   return m_rgba;
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
   cColor(const float rgba[4]) : cColorImpl<float>(rgba) {}
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
