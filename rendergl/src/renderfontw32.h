///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTW32_H
#define INCLUDED_RENDERFONTW32_H

#include "renderfontapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextureRenderFontW32
//

struct sTextureFontGlyph
{
   float texCoords[4];
   uint width;
   int lead, trail;
};

class cTextureRenderFontW32 : public cComObject<IMPLEMENTS(IRenderFont)>
{
   cTextureRenderFontW32(const cTextureRenderFontW32 &); // un-implemented
   void operator=(const cTextureRenderFontW32 &); // un-implemented

public:
   cTextureRenderFontW32();
   ~cTextureRenderFontW32();

   virtual void OnFinalRelease();

   bool Create(const tChar * pszFontName, int pointSize, bool bBold, bool bItalic);

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const float color[4]) const;

   tResult SetDropShadowState(float offsetX, float offsetY, const float color[4]);

private:
   int m_texDim; // texture is always square
   int m_rowHeight; // height of a row of characters in the texture (all rows are same height)
   sTextureFontGlyph * m_pGlyphs;
   uint m_texId;
   float m_widthMult, m_heightMult;
   float m_dropShadowOffsetX, m_dropShadowOffsetY;
   float m_dropShadowColor[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTW32_H
