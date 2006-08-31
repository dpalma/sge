///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTD3D_H
#define INCLUDED_GUIFONTD3D_H

#include "guiapi.h"
#include "color.h"

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ID3DXFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontD3D
//

class cGUIFontD3D : public cComObject<IMPLEMENTS(IGUIFont)>
{
   cGUIFontD3D(const cGUIFontD3D &); // un-implemented
   void operator=(const cGUIFontD3D &); // un-implemented

public:
   cGUIFontD3D(ID3DXFont * pD3dxFont);
   ~cGUIFontD3D();

   virtual tResult RenderText(const char * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;
   virtual tResult RenderText(const wchar_t * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;

private:
#if HAVE_DIRECTX
   mutable cAutoIPtr<ID3DXFont> m_pD3dxFont;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTD3D_H
