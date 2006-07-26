////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_FREETYPEUTILS_H
#define INCLUDED_FREETYPEUTILS_H

#include "comtools.h"

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

   tResult ToBitmap(FT_Render_Mode renderMode, bool bDestroy, FT_BitmapGlyph * pBitmap);

private:
   FT_Glyph m_glyph;
};


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
