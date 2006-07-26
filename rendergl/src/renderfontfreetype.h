////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTFREETYPE_H
#define INCLUDED_RENDERFONTFREETYPE_H

#include "renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef unsigned int GLenum;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderFontFreetype
//

struct sTextureFontGlyph
{
   float texCoords[4];
   uint width;
   int lead, trail;
};

class cRenderFontFreetype : public cComObject<IMPLEMENTS(IRenderFont)>
{
   cRenderFontFreetype(const cRenderFontFreetype &);
   const cRenderFontFreetype & operator =(const cRenderFontFreetype &);

   cRenderFontFreetype(uint textureId, sTextureFontGlyph * pGlyphs);

public:
   virtual ~cRenderFontFreetype();

   static tResult Create(const tChar * pszFont, int fontPointSize, IRenderFont * * ppFont);

   virtual tResult MeasureText(const tChar * pszText, int textLength, uint flags, int * pWidth, int * pHeight) const;

   virtual tResult RenderText(const tChar * pszText, int textLength, uint flags, int x, int y, int width, int height) const;

private:
   uint m_textureId;
   sTextureFontGlyph * m_pGlyphs;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTFREETYPE_H
