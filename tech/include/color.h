///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#define ARGB(a,r,g,b) \
   (((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

class cColor
{
public:
   cColor();
   cColor(float r, float g, float b);
   cColor(float r, float g, float b, float a);
   cColor(const float rgba[4]);
   cColor(const cColor & other);
   const cColor & operator =(const cColor & other);

   bool operator !=(const cColor & other);
   bool operator ==(const cColor & other);

   float GetRed() const;
   float GetGreen() const;
   float GetBlue() const;
   float GetAlpha() const;

   uint GetARGB();

   const float * GetPointer() const;

private:
   float m_rgba[4];
   mutable bool m_bHaveARGB;
   mutable uint m_argb;
};

///////////////////////////////////////

inline cColor::cColor()
 : m_bHaveARGB(false),
   m_argb(0)
{
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b)
 : m_bHaveARGB(false),
   m_argb(0)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = 1;
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b, float a)
 : m_bHaveARGB(false),
   m_argb(0)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = a;
}

///////////////////////////////////////

inline cColor::cColor(const float rgba[4])
 : m_bHaveARGB(false),
   m_argb(0)
{
   m_rgba[0] = rgba[0];
   m_rgba[1] = rgba[1];
   m_rgba[2] = rgba[2];
   m_rgba[3] = rgba[3];
}

///////////////////////////////////////

inline cColor::cColor(const cColor & other)
 : m_bHaveARGB(other.m_bHaveARGB),
   m_argb(other.m_argb)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
}

///////////////////////////////////////

inline const cColor & cColor::operator =(const cColor & other)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
   m_bHaveARGB = other.m_bHaveARGB;
   m_argb = other.m_argb;
   return *this;
}

///////////////////////////////////////

inline bool cColor::operator !=(const cColor & other)
{
   return
      (m_rgba[0] != other.m_rgba[0]) &&
      (m_rgba[1] != other.m_rgba[1]) &&
      (m_rgba[2] != other.m_rgba[2]) &&
      (m_rgba[3] != other.m_rgba[3]);
}

///////////////////////////////////////

inline bool cColor::operator ==(const cColor & other)
{
   return
      (m_rgba[0] == other.m_rgba[0]) &&
      (m_rgba[1] == other.m_rgba[1]) &&
      (m_rgba[2] == other.m_rgba[2]) &&
      (m_rgba[3] == other.m_rgba[3]);
}

///////////////////////////////////////

inline float cColor::GetRed() const
{
   return m_rgba[0];
}

///////////////////////////////////////

inline float cColor::GetGreen() const
{
   return m_rgba[1];
}

///////////////////////////////////////

inline float cColor::GetBlue() const
{
   return m_rgba[2];
}

///////////////////////////////////////

inline float cColor::GetAlpha() const
{
   return m_rgba[3];
}

///////////////////////////////////////

inline uint cColor::GetARGB()
{
   if (!m_bHaveARGB)
   {
      m_bHaveARGB = true;
      m_argb = ARGB((byte)(GetAlpha() * 255),
                    (byte)(GetRed() * 255),
                    (byte)(GetGreen() * 255),
                    (byte)(GetBlue() * 255));
   }
   return m_argb;
}

///////////////////////////////////////

inline const float * cColor::GetPointer() const
{
   return m_rgba;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
