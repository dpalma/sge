///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#include "renderdll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

class RENDER_API cColor
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

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
