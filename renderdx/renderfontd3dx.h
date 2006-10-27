///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTD3D_H
#define INCLUDED_GUIFONTD3D_H

#include "render/renderfontapi.h"
#include "tech/color.h"

#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ID3DXFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderFontD3DX
//

class cRenderFontD3DX : public cComObject<IMPLEMENTS(IRenderFont)>
{
   cRenderFontD3DX(const cRenderFontD3DX &); // un-implemented
   void operator=(const cRenderFontD3DX &); // un-implemented

public:
   cRenderFontD3DX(ID3DXFont * pD3dxFont);
   ~cRenderFontD3DX();

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect,
                              uint flags, const float color[4]) const;

private:
   mutable cAutoIPtr<ID3DXFont> m_pD3dxFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTD3D_H
