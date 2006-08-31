///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTAPI_H
#define INCLUDED_RENDERFONTAPI_H

#include "renderdll.h"
#include "comtools.h"

#include "color.h"
#include "rect.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderFont
//

interface IRenderFont : IUnknown
{
   virtual tResult MeasureText(const tChar * pszText, int textLength, int * pWidth, int * pHeight) const = 0;

   virtual tResult RenderText(const tChar * pszText, int textLength, int x, int y) const = 0;
};

///////////////////////////////////////

RENDER_API tResult RenderFontCreate(const tChar * pszFont, int fontPointSize, IUnknown * pUnk, IRenderFont * * ppFont);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTAPI_H
