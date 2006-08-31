///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIFONTW32_H
#define INCLUDED_GUIFONTW32_H

#include "guiapi.h"
#include "color.h"

#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUITextureFontW32
//

struct sGUITextureFontGlyph
{
   float texCoords[4];
   uint width;
   int lead, trail;
};

class cGUITextureFontW32 : public cComObject<IMPLEMENTS(IGUIFont)>
{
   cGUITextureFontW32(const cGUITextureFontW32 &); // un-implemented
   void operator=(const cGUITextureFontW32 &); // un-implemented

public:
   cGUITextureFontW32();
   ~cGUITextureFontW32();

   virtual void OnFinalRelease();

   bool Create(const tChar * pszFontName, int pointSize, bool bBold, bool bItalic);

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const;

   tResult SetDropShadowState(float offsetX, float offsetY, const cColor & color);

private:
   int m_texDim; // texture is always square
   int m_rowHeight; // height of a row of characters in the texture (all rows are same height)
   sGUITextureFontGlyph * m_pGlyphs;
   uint m_texId;
   float m_widthMult, m_heightMult;
   float m_dropShadowOffsetX, m_dropShadowOffsetY;
   cColor m_dropShadowColor;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIFONTW32_H
