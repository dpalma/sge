///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTFREETYPE_H
#define INCLUDED_GUIFONTFREETYPE_H

#include "guiapi.h"

#include "FTGLTextureFont.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIFontFreetype
//

class cGUIFontFreetype : public cComObject<IMPLEMENTS(IGUIFont)>
{
   cGUIFontFreetype(const cGUIFontFreetype &); // un-implemented
   const cGUIFontFreetype & operator=(const cGUIFontFreetype &); // un-implemented

public:
   cGUIFontFreetype();
   ~cGUIFontFreetype();

   virtual void OnFinalRelease();

   tResult Create(const cGUIFontDesc & fontDesc);

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;

private:
   FTGLTextureFont * m_pFont;
   cAutoIPtr<IRenderFont> m_pRenderFont;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTFREETYPE_H
