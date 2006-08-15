////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderfontfreetype.h"
#include "freetypeutils.h"

#include "color.h"
#include "filepath.h"
#include "filespec.h"

#include <GL/glew.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#endif

#include <cstring>

#include "dbgalloc.h" // must be last header


#define kRenderFontGlyphFirst 32
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
// CLASS: cRenderFontFreetype
//

////////////////////////////////////////

cRenderFontFreetype::cRenderFontFreetype(uint textureId, int textureSize, sTextureFontGlyph * pGlyphs)
 : m_textureId(textureId)
 , m_textureSize(textureSize)
 , m_pGlyphs(pGlyphs)
{
}

////////////////////////////////////////

cRenderFontFreetype::~cRenderFontFreetype()
{
   glDeleteTextures(1, &m_textureId);
   m_textureId = 0;

   if (m_pGlyphs != NULL)
   {
      delete [] m_pGlyphs;
      m_pGlyphs = NULL;
   }
}

////////////////////////////////////////

tResult cRenderFontFreetype::Create(const tChar * pszFont, int fontPointSize, IRenderFont * * ppFont)
{
   if (pszFont == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   if (fontPointSize <= 4)
   {
      ErrorMsg1("Insanely small point size (%d) requested\n", fontPointSize);
      return E_INVALIDARG;
   }

   if (fontPointSize > 50)
   {
      ErrorMsg1("Insanely large point size (%d) requested\n", fontPointSize);
      return E_INVALIDARG;
   }

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
      float pixelsH = static_cast<float>(GetDeviceCaps(hMemDC, HORZRES));
      int mmH = GetDeviceCaps(hMemDC, HORZSIZE);
      int dpiH = FloatToInt((pixelsH * 25.4f) / mmH);
      float pixelsV = static_cast<float>(GetDeviceCaps(hMemDC, VERTRES));
      int mmV = GetDeviceCaps(hMemDC, VERTSIZE);
      int dpiV = FloatToInt((pixelsV * 25.4f) / mmV);

      // FreeType measures fonts in 64ths of a point
      face.SetCharSize(0, fontPointSize * 64, dpiH, dpiV);

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

   sTextureFontGlyph * pGlyphs = new sTextureFontGlyph[kRenderFontGlyphCount];
   if (pGlyphs == NULL)
   {
      return E_OUTOFMEMORY;
   }

   int texX = 0, texY = 0;

   static const int kPadHorz = 1, kPadVert = 1;

   int maxHeightThisRow = 0;

   FT_BitmapGlyph bitmapGlyphs[kRenderFontGlyphCount];
   for (int c = kRenderFontGlyphFirst; c <= kRenderFontGlyphLast; ++c)
   {
      uint index = c - kRenderFontGlyphFirst;

      cFreetypeGlyph glyph;
      if (face.LoadGlyph(face.GetCharIndex(c), FT_LOAD_DEFAULT, &glyph) == S_OK)
      {
         float minX, minY, maxX, maxY;
         if (glyph.BBox(&minX, &minY, &maxX, &maxY) == S_OK
            && glyph.ToBitmap(FT_RENDER_MODE_NORMAL, true, &bitmapGlyphs[index]) == S_OK)
         {
            FT_Bitmap & bitmap = bitmapGlyphs[index]->bitmap;

            if ((texX + bitmap.width + kPadHorz) > texSize)
            {
               texY += maxHeightThisRow + kPadVert;
               texX = 0;
               maxHeightThisRow = 0;
            }

            for (int j = 0; j < bitmap.rows; ++j)
            {
               for (int i = 0; i < bitmap.width; ++i)
               {
                  byte value = bitmap.buffer[i + (bitmap.pitch * j)];
                  uint texelIndex = texX + i + ((texY + j) * texSize);
                  pTexData[2 * texelIndex] = pTexData[2 * texelIndex + 1] = value;
               }
            }

            pGlyphs[index].texCoords[0] = static_cast<float>(texX) / texSize;
            pGlyphs[index].texCoords[1] = static_cast<float>(texY) / texSize;
            pGlyphs[index].texCoords[2] = static_cast<float>(texX + bitmap.width) / texSize;
            pGlyphs[index].texCoords[3] = static_cast<float>(texY + bitmap.rows) / texSize;

            pGlyphs[index].minX = minX;
            pGlyphs[index].minY = minY;
            pGlyphs[index].maxX = maxX;
            pGlyphs[index].maxY = maxY;

            pGlyphs[index].advanceX = glyph.AdvanceX();
            pGlyphs[index].advanceY = glyph.AdvanceY();

            if (bitmap.rows > maxHeightThisRow)
            {
               maxHeightThisRow = bitmap.rows;
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
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   delete [] pTexData;

   *ppFont = static_cast<IRenderFont *>(new cRenderFontFreetype(textureId, texSize, pGlyphs));
   if (*ppFont == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFreetype::MeasureText(const tChar * pszText, int textLength, int * pWidth, int * pHeight) const
{
   if (textLength < 0)
   {
      textLength = _tcslen(pszText);
   }

   for (int i = 0; i < textLength; i++)
   {
      tChar c = pszText[i];

      uint index = c - kRenderFontGlyphFirst;

      const sTextureFontGlyph & glyph = m_pGlyphs[index];
   }

   return E_NOTIMPL;
}

////////////////////////////////////////

tResult cRenderFontFreetype::RenderText(const tChar * pszText, int textLength, int x, int y) const
{
   if (textLength < 0)
   {
      textLength = _tcslen(pszText);
   }

   sTextVertex * vertices = reinterpret_cast<sTextVertex *>(alloca(6 * textLength * sizeof(sTextVertex)));
   uint nVertices = 0;

   float tx = static_cast<float>(x), ty = static_cast<float>(y);

   for (int i = 0; i < textLength; i++)
   {
      tChar c = pszText[i];

      uint index = c - kRenderFontGlyphFirst;

      const sTextureFontGlyph & glyph = m_pGlyphs[index];

      float tx1 = glyph.texCoords[0];
      float ty1 = glyph.texCoords[1];
      float tx2 = glyph.texCoords[2];
      float ty2 = glyph.texCoords[3];

      int th = FloatToInt((ty2 - ty1) * m_textureSize);

      if (c != _T(' '))
      {
         sTextVertex * pVertex = &vertices[nVertices];
         float xPlusW = tx + glyph.maxX;

         // Triangles go in counter-clockwise order:
         //
         // 1  4----6
         // |\  \   |
         // | \  \  |
         // |  \  \ |
         // |   \  \|
         // 2----3  5

         // 1: top
         pVertex->u = tx1;
         pVertex->v = ty1;
         pVertex->x = tx;
         pVertex->y = ty - glyph.maxY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;

         // 2: bottom left
         pVertex->u = tx1;
         pVertex->v = ty2;
         pVertex->x = tx;
         pVertex->y = ty - glyph.minY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;

         // 3: bottom right
         pVertex->u = tx2;
         pVertex->v = ty2;
         pVertex->x = xPlusW;
         pVertex->y = ty - glyph.minY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;

         // 4: top left
         pVertex->u = tx1;
         pVertex->v = ty1;
         pVertex->x = tx;
         pVertex->y = ty - glyph.maxY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;

         // 5: bottom
         pVertex->u = tx2;
         pVertex->v = ty2;
         pVertex->x = xPlusW;
         pVertex->y = ty - glyph.minY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;

         // 6: top right
         pVertex->u = tx2;
         pVertex->v = ty1;
         pVertex->x = xPlusW;
         pVertex->y = ty - glyph.maxY;
         pVertex->z = 0;
         pVertex++;
         nVertices++;
      }

      tx += glyph.advanceX;
   }

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, m_textureId);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glInterleavedArrays(GL_T2F_V3F, 0, vertices);

   glDrawArrays(GL_TRIANGLES, 0, nVertices);

   return S_OK;
}

////////////////////////////////////////

tResult RenderFontCreate(const tChar * pszFont, int fontPointSize, IUnknown * pUnk, IRenderFont * * ppFont)
{
   return cRenderFontFreetype::Create(pszFont, fontPointSize, ppFont);
}

////////////////////////////////////////////////////////////////////////////////
