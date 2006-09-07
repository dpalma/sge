////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTFREETYPE_H
#define INCLUDED_RENDERFONTFREETYPE_H

#include "renderfontapi.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef unsigned int GLenum;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderFontFreetype
//

class cRenderFontFreetype : public cComObject<IMPLEMENTS(IRenderFont)>
{
   cRenderFontFreetype(const cRenderFontFreetype &);
   const cRenderFontFreetype & operator =(const cRenderFontFreetype &);

   struct sTextureFontGlyph
   {
      float texCoords[4];
      float minX, minY, maxX, maxY;
      float advanceX, advanceY;
   };

   // GL_T2F_V3F format
   struct sTextVertex
   {
      float u, v;
      float x, y, z;
   };

   cRenderFontFreetype(uint textureId, int textureSize, sTextureFontGlyph * pGlyphs);

public:
   virtual ~cRenderFontFreetype();

   static tResult Create(const tChar * pszFont, int fontPointSize, IRenderFont * * ppFont);

   virtual tResult RenderText(const tChar * pszText, int textLength, tRect * pRect,
                              uint flags, const float color[4]) const;

private:
   uint m_textureId;
   int m_textureSize; // texture is square so only one dimension
   sTextureFontGlyph * m_pGlyphs;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTFREETYPE_H
