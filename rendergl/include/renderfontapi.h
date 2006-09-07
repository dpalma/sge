///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTAPI_H
#define INCLUDED_RENDERFONTAPI_H

#include "renderdll.h"
#include "comtools.h"

#include "rect.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

enum
{
   kASCIIGlyphFirst = 32,
   kASCIIGlyphLast = 128,
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderFont
//

enum eRenderFontFlags
{
   kRFF_None      = (0<<0),
   kRFF_Bold      = (1<<0),
   kRFF_Italic    = (1<<1),
   kRFF_Shadow    = (1<<2),
   kRFF_Outline   = (1<<3),
};

enum eRenderTextFlags
{
   kRT_Center        = (1<<0),
   kRT_VCenter       = (1<<1),
   kRT_NoClip        = (1<<2),
   kRT_CalcRect      = (1<<3),
   kRT_SingleLine    = (1<<4),
   kRT_Bottom        = (1<<5),
   kRT_NoBlend       = (1<<6),
   kRT_DropShadow    = (1<<7),
};

interface IRenderFont : IUnknown
{
   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect,
                              uint flags, const float color[4]) const = 0;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTAPI_H
