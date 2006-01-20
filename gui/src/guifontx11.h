///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTX11_H
#define INCLUDED_GUIFONTX11_H

#include "guiapi.h"
#include "color.h"

#include "comtools.h"

#include <X11/Xlib.h>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGLXRasterFont
//

class cGLXRasterFont : public cComObject<IMPLEMENTS(IGUIFont)>
{
public:
   cGLXRasterFont();
   virtual ~cGLXRasterFont();

   virtual void OnFinalRelease();

   virtual tResult RenderText(const char * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;
   virtual tResult RenderText(const wchar_t * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;

   bool Create(const cGUIFontDesc & fontDesc);

private:
   cGLXRasterFont(const cGLXRasterFont &);
   const cGLXRasterFont & operator=(const cGLXRasterFont &);

   float GetHeight() const { return m_height; }

   XFontStruct * m_pFontInfo;
   Display * m_pDisplay;

   int m_glyphStart;
   int m_listCount;
   int m_listBase;

   float m_height;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTX11_H
