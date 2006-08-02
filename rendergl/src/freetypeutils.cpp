////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "freetypeutils.h"

#include <freetype/ftbbox.h>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeGlyph
//

////////////////////////////////////////

cFreetypeGlyph::cFreetypeGlyph()
 : m_glyph(NULL)
{
}

////////////////////////////////////////

cFreetypeGlyph::~cFreetypeGlyph()
{
   FT_Done_Glyph(m_glyph);
}

////////////////////////////////////////

tResult cFreetypeGlyph::BBox(float * pLowerX, float * pLowerY, float * pUpperX, float * pUpperY)
{
   if (pLowerX == NULL || pLowerY == NULL || pUpperX == NULL || pUpperY == NULL)
   {
      return E_POINTER;
   }

   *pLowerX = static_cast<float>(m_bbox.xMin) / 64.0f;
   *pLowerY = static_cast<float>(m_bbox.yMin) / 64.0f;
   *pUpperX = static_cast<float>(m_bbox.xMax) / 64.0f;
   *pUpperY = static_cast<float>(m_bbox.yMax) / 64.0f;
   return S_OK;
}

////////////////////////////////////////

tResult cFreetypeGlyph::ToBitmap(FT_Render_Mode renderMode, bool bDestroy, FT_BitmapGlyph * pBitmap)
{
   if (pBitmap == NULL)
   {
      return E_POINTER;
   }

   if (FT_Glyph_To_Bitmap(&m_glyph, renderMode, NULL, bDestroy) != FT_Err_Ok)
   {
      return E_FAIL;
   }

   *pBitmap = (FT_BitmapGlyph)m_glyph;
   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeFace
//

////////////////////////////////////////

cFreetypeFace::cFreetypeFace()
 : m_face(NULL)
{
}

////////////////////////////////////////

cFreetypeFace::~cFreetypeFace()
{
   FT_Done_Face(m_face);
}

////////////////////////////////////////

tResult cFreetypeFace::SetCharSize(long width, long height, uint hres, uint vres)
{
   return (FT_Set_Char_Size(m_face, width, height, hres, vres) == FT_Err_Ok) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cFreetypeFace::SetPixelSize(uint width, uint height)
{
   return (FT_Set_Pixel_Sizes(m_face, width, height) == FT_Err_Ok) ? S_OK : E_FAIL;
}

////////////////////////////////////////

uint cFreetypeFace::GetCharIndex(ulong c)
{
   return FT_Get_Char_Index(m_face, c);
}

////////////////////////////////////////

tResult cFreetypeFace::LoadGlyph(uint index, int flags, cFreetypeGlyph * pGlyph)
{
   if (pGlyph == NULL)
   {
      return E_POINTER;
   }

   if (FT_Load_Glyph(m_face, index, flags) == FT_Err_Ok
      && FT_Get_Glyph(m_face->glyph, &pGlyph->m_glyph) == FT_Err_Ok
      && FT_Outline_Get_BBox(&m_face->glyph->outline, &pGlyph->m_bbox) == FT_Err_Ok)
   {
      return S_OK;
   }

   return E_FAIL;
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFreetypeLibrary
//

////////////////////////////////////////

cFreetypeLibrary::cFreetypeLibrary()
 : m_bInitSuccess(false)
{
   m_bInitSuccess = (FT_Init_FreeType(&m_library) == FT_Err_Ok);
}

////////////////////////////////////////

cFreetypeLibrary::~cFreetypeLibrary()
{
   FT_Done_FreeType(m_library);
}

////////////////////////////////////////

tResult cFreetypeLibrary::NewFace(const tChar * pszFace, cFreetypeFace * pFace)
{
   return NewFace(pszFace, 0, pFace);
}

////////////////////////////////////////

tResult cFreetypeLibrary::NewFace(const tChar * pszFace, long faceIndex, cFreetypeFace * pFace)
{
   if (pszFace == NULL || pFace == NULL)
   {
      return E_POINTER;
   }

   if (!IsInitialized())
   {
      return E_FAIL;
   }

   if (FT_New_Face(m_library, pszFace, faceIndex, &pFace->m_face) != FT_Err_Ok)
   {
      return E_FAIL;
   }

   return S_OK;
}


////////////////////////////////////////////////////////////////////////////////
