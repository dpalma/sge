///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTX11_H
#define INCLUDED_RENDERFONTX11_H

#include "renderfontapi.h"

#include <X11/Xlib.h>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGLXRasterFont
//

class cGLXRasterFont : public cComObject<IMPLEMENTS(IRenderFont)>
{
public:
   cGLXRasterFont();
   virtual ~cGLXRasterFont();

   virtual void OnFinalRelease();

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const float color[4]) const;

   bool Create(const tChar * pszFontName, int pointSize, bool bBold, bool bItalic);

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

#endif // !INCLUDED_RENDERFONTX11_H
