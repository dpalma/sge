///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#ifdef _MSC_VER
#pragma once
#endif

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

   const float * GetPointer() const;

private:
   float m_rgba[4];
};

///////////////////////////////////////

inline cColor::cColor()
{
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = 1;
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b, float a)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = a;
}

///////////////////////////////////////

inline cColor::cColor(const float rgba[4])
{
   m_rgba[0] = rgba[0];
   m_rgba[1] = rgba[1];
   m_rgba[2] = rgba[2];
   m_rgba[3] = rgba[3];
}

///////////////////////////////////////

inline cColor::cColor(const cColor & other)
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
   return *this;
}

///////////////////////////////////////

inline const float * cColor::GetPointer() const
{
   return m_rgba;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
