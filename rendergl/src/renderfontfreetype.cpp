////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderfontfreetype.h"

#include "color.h"
#include "filepath.h"
#include "filespec.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <GL/glew.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header


#define kRenderFontGlyphFirst 0
#define kRenderFontGlyphLast 127
#define kRenderFontGlyphCount (kRenderFontGlyphLast - kRenderFontGlyphFirst + 1)


///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
static tResult GetFontPath(cFilePath * pFontPath)
{
   if (pFontPath == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   typedef tResult (STDCALL * tSHGetFolderPath)(HWND, int, HANDLE, DWORD, LPTSTR);

   HMODULE hSHFolder = LoadLibrary(_T("SHFolder.dll"));
   if (hSHFolder != NULL)
   {
      tSHGetFolderPath pfn = reinterpret_cast<tSHGetFolderPath>(GetProcAddress(hSHFolder,
#ifdef _UNICODE
         "SHGetFolderPathW"));
#else
         "SHGetFolderPathA"));
#endif
      if (pfn != NULL)
      {
         tChar szPath[MAX_PATH];
         if ((*pfn)(NULL, CSIDL_FONTS, NULL, 0, szPath) == S_OK)
         {
            *pFontPath = cFilePath(szPath);
            result = S_OK;
         }
      }

      FreeLibrary(hSHFolder);
   }

   return result;
}
#else
#error ("TODO: Need platform-specific font path function")
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
      && FT_Get_Glyph(m_face->glyph, &pGlyph->m_glyph) == FT_Err_Ok)
   {
      return S_OK;
   }

   return E_FAIL;
}


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
//
// CLASS: cRenderFontFreetype
//

////////////////////////////////////////

cRenderFontFreetype::cRenderFontFreetype(uint textureId)
 : m_textureId(textureId)
{
}

////////////////////////////////////////

cRenderFontFreetype::~cRenderFontFreetype()
{
   glDeleteTextures(1, &m_textureId);
   m_textureId = 0;
}

////////////////////////////////////////

tResult cRenderFontFreetype::Create(const tChar * pszFont, int fontPointSize, IRenderFont * * ppFont)
{
   cFreetypeLibrary freetype;

   cFileSpec fontName(pszFont);
   fontName.SetFileExt(_T("ttf"));

   cFilePath fontPath;
   if (GetFontPath(&fontPath) == S_OK)
   {
      fontName.SetPath(fontPath);
   }

   cFreetypeFace face;
   if (freetype.NewFace(fontName.CStr(), &face) != S_OK)
   {
      return E_FAIL;
   }

   int maxTexSize = -1;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
   if (maxTexSize == -1)
   {
      WarnMsg("Unable to determine the maximum GL texture size\n");
      return E_FAIL;
   }

#ifdef _WIN32
   HDC hMemDC = CreateCompatibleDC(NULL);
   if (hMemDC != NULL)
   {
      int height = MulDiv(fontPointSize, GetDeviceCaps(hMemDC, LOGPIXELSY), 72);

      // FreeType measures fonts in 1/64ths of a pixel
      face.SetCharSize(height * 64, height * 64, 96, 96);

      DeleteDC(hMemDC);
   }
#else
#error ("TODO: platform-specific part of font creation")
#endif

   int texSize = Min(512, maxTexSize);

   // 2 bytes per pixel: luminance + alpha
   byte * pTexData = new byte[2 * texSize * texSize];
   if (pTexData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   // Fill with black to start
   memset(pTexData, 0, 2 * texSize * texSize);

   int texX = 0, texY = 0;

   static const int kPadHorz = 1, kPadVert = 1;

   FT_BitmapGlyph bitmapGlyphs[kRenderFontGlyphCount];
   for (int c = kRenderFontGlyphFirst; c <= kRenderFontGlyphLast; ++c)
   {
      uint index = c - kRenderFontGlyphFirst;

      cFreetypeGlyph glyph;
      if (face.LoadGlyph(face.GetCharIndex(c), FT_LOAD_DEFAULT, &glyph) == S_OK)
      {
         if (glyph.ToBitmap(FT_RENDER_MODE_NORMAL, true, &bitmapGlyphs[index]) == S_OK)
         {
            FT_Bitmap & bitmap = bitmapGlyphs[index]->bitmap;

            if ((texX + bitmap.width + kPadHorz) > texSize)
            {
               texX = 0;
               texY += bitmap.rows + kPadVert;
            }

            for (int j = 0; j < bitmap.rows; ++j)
            {
               for (int i = 0; i < bitmap.width; ++i)
               {
                  byte value = bitmap.buffer[i + (bitmap.width * j)];
                  uint texelIndex = texX + i + ((texY + j) * texSize);
                  pTexData[2 * texelIndex] = pTexData[2 * texelIndex + 1] = value;
               }
            }

            texX += bitmap.width + kPadHorz;
         }
      }
   }

   uint textureId = 0;
   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, texSize, texSize, 0,
                GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pTexData);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   delete [] pTexData;

   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFreetype::MeasureText(const tChar * pszText, int textLength, uint flags, int * pWidth, int * pHeight) const
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderFontFreetype::RenderText(const tChar * pszText, int textLength, uint flags, int x, int y, int width, int height)
{
   return E_NOTIMPL;
}

////////////////////////////////////////

tResult RenderFontCreate(const tChar * pszFont, int fontPointSize, IUnknown * pUnk, IRenderFont * * ppFont)
{
   return cRenderFontFreetype::Create(pszFont, fontPointSize, ppFont);
}

////////////////////////////////////////////////////////////////////////////////
