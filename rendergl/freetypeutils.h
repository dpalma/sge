////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FREETYPEUTILS_H
#define INCLUDED_FREETYPEUTILS_H

#include "tech/comtools.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeGlyph
//

class cFreetypeGlyph
{
   friend class cFreetypeFace;

public:
   cFreetypeGlyph();
   ~cFreetypeGlyph();

   float AdvanceX() const;
   float AdvanceY() const;

   tResult BBox(float * pMinX, float * pMinY, float * pMaxX, float * pMaxY);

   tResult ToBitmap(FT_Render_Mode renderMode, bool bDestroy, FT_BitmapGlyph * pBitmap);

private:
   FT_Glyph m_glyph;
   FT_BBox m_bbox;
};

////////////////////////////////////////

inline float cFreetypeGlyph::AdvanceX() const
{
   return static_cast<float>(m_glyph->advance.x) / 65536.0f;
}

////////////////////////////////////////

inline float cFreetypeGlyph::AdvanceY() const
{
   return static_cast<float>(m_glyph->advance.y) / 65536.0f;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeFace
//

class cFreetypeFace
{
   friend class cFreetypeLibrary;

public:
   cFreetypeFace();
   ~cFreetypeFace();

   tResult SetCharSize(long width, long height, uint hres, uint vres);

   tResult SetPixelSize(uint width, uint height);

   uint GetCharIndex(ulong c);

   tResult LoadGlyph(uint index, int flags, cFreetypeGlyph * pGlyph);

private:
   FT_Face m_face;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeLibrary
//

class cFreetypeLibrary
{
   cFreetypeLibrary(const cFreetypeLibrary &);
   const cFreetypeLibrary & operator =(const cFreetypeLibrary &);

public:
   cFreetypeLibrary();
   ~cFreetypeLibrary();

   bool IsInitialized() const { return m_bInitSuccess; }

   tResult NewFace(const tChar * pszFace, cFreetypeFace * pFace);
   tResult NewFace(const tChar * pszFace, long faceIndex, cFreetypeFace * pFace);

private:
   FT_Library m_library;
   bool m_bInitSuccess;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_FREETYPEUTILS_H
