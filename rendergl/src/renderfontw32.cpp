///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "renderfontw32.h"

#include "colortem.h"
#include "configapi.h"
#include "techmath.h"

#ifndef _WIN32
#error ("This file is for Windows compilation only")
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <GL/glew.h>

#include <cstring>

#include "dbgalloc.h" // must be last header

// REFERENCES
// http://www.opengl.org/resources/code/rendering/mjktips/TexFont/TexFont.html

///////////////////////////////////////////////////////////////////////////////

#define IsBitFlagSet(var, bit) (((var) & (bit)) == (bit))

///////////////////////////////////////////////////////////////////////////////

const uint kDropShadowStencilRef = 0x7f;
const uint kDropShadowStencilMask = 0xff;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextureRenderFontW32
//

///////////////////////////////////////

cTextureRenderFontW32::cTextureRenderFontW32()
 : m_texDim(0)
 , m_pGlyphs(NULL)
 , m_texId(0)
 , m_widthMult(1)
 , m_heightMult(1)
 , m_dropShadowOffsetX(1)
 , m_dropShadowOffsetY(1)
 , m_dropShadowColor(0,0,0)
{
}

///////////////////////////////////////

cTextureRenderFontW32::~cTextureRenderFontW32()
{
}

///////////////////////////////////////

void cTextureRenderFontW32::OnFinalRelease()
{
   glDeleteTextures(1, &m_texId);
   m_texDim = 0;
   m_texId = 0;
   delete [] m_pGlyphs;
   m_pGlyphs = NULL;
}

///////////////////////////////////////

static uint ShiftFromMask(uint mask)
{
   uint shift = 0;
   while (!(mask & 1))
   {
      mask >>= 1;
      shift++;
   }
   return shift;
}

static int GetTextureSizeForFont(int pointSize)
{
   if (pointSize > 40)
   {
      return 1024;
   }
   else if (pointSize > 20)
   {
      return 512;
   }
   else
   {
      return 256;
   }
}

bool cTextureRenderFontW32::Create(const tChar * pszFontName, int pointSize, bool bBold, bool bItalic)
{
   if (pszFontName == NULL || pointSize < 0)
   {
      return false;
   }

   m_texDim = GetTextureSizeForFont(pointSize);

   int maxTexSize = -1;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
   if (maxTexSize == -1)
   {
      return false;
   }

   if (m_texDim < 0)
   {
      m_texDim = maxTexSize;
   }

   bool bSucceeded = false;

//#ifdef _DEBUG
//   maxTexSize = 128;
//#endif

   float scale = 1;
   float oneOverScale = 1;

   if (m_texDim > maxTexSize)
   {
      scale = (float)maxTexSize / m_texDim;
      oneOverScale = (float)m_texDim / maxTexSize;
      m_texDim = maxTexSize;
   }

   m_widthMult = m_texDim * oneOverScale;
   m_heightMult = m_texDim * oneOverScale;

   HDC hMemDC = CreateCompatibleDC(NULL);
   if (hMemDC != NULL)
   {
      BITMAPINFO bmi;
      memset(&bmi.bmiHeader, 0, sizeof(bmi.bmiHeader));
      bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
      bmi.bmiHeader.biWidth = m_texDim;
      bmi.bmiHeader.biHeight = -m_texDim;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.bmiHeader.biBitCount = 16;

      uint redMask = 0, greenMask = 0, blueMask = 0;
      uint redShift = 0, greenShift = 0, blueShift = 0;

      uint * pBitmapBits = NULL;
      HBITMAP hBitmap = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
      if (hBitmap != NULL)
      {
         HGDIOBJ hOldBitmap = SelectObject(hMemDC, hBitmap);

         DIBSECTION dibSection = {0};
         Verify(GetObject(hBitmap, sizeof(dibSection), &dibSection));

         redMask = dibSection.dsBitfields[0];
         greenMask = dibSection.dsBitfields[1];
         blueMask = dibSection.dsBitfields[2];

         redShift = ShiftFromMask(redMask);
         greenShift = ShiftFromMask(greenMask);
         blueShift = ShiftFromMask(blueMask);

         LOGFONT lf = {0};
         lf.lfHeight = -MulDiv(pointSize, (int)(GetDeviceCaps(hMemDC, LOGPIXELSY) * scale), 72);
         lf.lfWeight = bBold ? FW_SEMIBOLD : FW_NORMAL;
         lf.lfItalic = bItalic;
         lf.lfQuality = ANTIALIASED_QUALITY;
         lstrcpyn(lf.lfFaceName, pszFontName, _countof(lf.lfFaceName));
         lf.lfFaceName[_countof(lf.lfFaceName) - 1] = 0;

         HFONT hFont = CreateFontIndirect(&lf);
         if (hFont != NULL)
         {
            HGDIOBJ hOldFont = SelectObject(hMemDC, hFont);

            static const COLORREF kWhite = RGB(255,255,255);
            static const COLORREF kBlack = RGB(0,0,0);
            static const COLORREF kBlue = RGB(0,0,255);

            SetMapMode(hMemDC, MM_TEXT);
            SetBkColor(hMemDC, kBlue);
            SetTextAlign(hMemDC, TA_TOP | TA_LEFT);
            SetBkMode(hMemDC, TRANSPARENT);

            float texX = 0, texY = 0;

            float oneOverTexDim = 1.0f / m_texDim;

            static const int kPadHorz = 1;
            static const int kPadVert = 1;

            int padVert = kPadVert;

            uint glyphCount = kASCIIGlyphLast - kASCIIGlyphFirst + 1;
            m_pGlyphs = new sTextureFontGlyph[glyphCount];

            // draw each character into the bitmap and compute its texture coordinates
            for (uint c = kASCIIGlyphFirst; c < kASCIIGlyphLast; c++)
            {
               char szChar[2];
               szChar[0] = c;
               szChar[1] = 0;

               SIZE extent;
               GetTextExtentPoint32(hMemDC, szChar, 1, &extent);

               ABC abc;
               if (!GetCharABCWidths(hMemDC, c, c, &abc))
               {
                  abc.abcA = 0;
                  abc.abcB = extent.cx;
                  abc.abcC = 0;
               }

               Assert(extent.cx == (abc.abcA + abc.abcB + abc.abcC));

               if ((texX + extent.cx + kPadHorz) > m_texDim)
               {
                  texX = 0;
                  texY += extent.cy + kPadVert;
               }

               SetTextColor(hMemDC, kWhite);
               ExtTextOut(hMemDC, FloatToInt(texX), FloatToInt(texY), 0, NULL, szChar, 1, NULL);

               uint index = c - kASCIIGlyphFirst;
               m_pGlyphs[index].texCoords[0] = texX * oneOverTexDim;
               m_pGlyphs[index].texCoords[1] = texY * oneOverTexDim;
               m_pGlyphs[index].texCoords[2] = (texX + (float)extent.cx) * oneOverTexDim;
               m_pGlyphs[index].texCoords[3] = (texY + (float)extent.cy) * oneOverTexDim;
               m_pGlyphs[index].width = abc.abcB;
               m_pGlyphs[index].lead = abc.abcA;
               m_pGlyphs[index].trail = abc.abcC;

               texX += extent.cx + kPadHorz;
            }

            bSucceeded = true;

            SelectObject(hMemDC, hOldFont);

            DeleteObject(hFont);
         }

         static const float kOneOver255 = 1.0f / 255.0f;

         uint16 * pBits = (uint16 *)pBitmapBits;
         for (int i = 0; i < (m_texDim * m_texDim); i++, pBits++)
         {
            float red = (float)((*pBits & redMask) >> redShift) * kOneOver255;
            float green = (float)((*pBits & greenMask) >> greenShift) * kOneOver255;
            float blue = (float)((*pBits & blueMask) >> blueShift) * kOneOver255;

            float luminance = (red * 0.3f) + (green * 0.59f) + (blue * 0.11f);

            float alpha = red + (1-blue); // Extract alpha from above

            float lum = red / alpha;     // Extract lum from above

            uint a = FloatToInt(alpha * 255);
            uint l = FloatToInt(lum * 255);

#define RGBA16(r,g,b,a) (uint16)(((r & 0xF) << 12) | ((g & 0xF) << 8) | ((b & 0xF) << 4) | (a & 0xF))

            *pBits = RGBA16(l,l,a,a);
         }

         SelectObject(hMemDC, hOldBitmap);

         if (bSucceeded)
         {
            glGenTextures(1, &m_texId);
            glBindTexture(GL_TEXTURE_2D, m_texId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, m_texDim, m_texDim, 0,
                         GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pBitmapBits);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         }

         DeleteObject(hBitmap);
      }

      DeleteDC(hMemDC);
   }

   if (bSucceeded)
   {
      m_rowHeight = FloatToInt((m_pGlyphs[0].texCoords[3] - m_pGlyphs[0].texCoords[1]) * m_texDim);
   }

   return bSucceeded;
}

///////////////////////////////////////

tResult cTextureRenderFontW32::RenderText(const tChar * pszText, int textLength, tRect * pRect,
                                          uint flags, const float color[4]) const
{
   if (pszText == NULL || pRect == NULL)
   {
      return E_POINTER;
   }

   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, m_texId);

   if (!IsBitFlagSet(flags, kRT_NoClip))
   {
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);

      glEnable(GL_SCISSOR_TEST);
      glScissor(
         pRect->left,
         // HACK: the call to glOrtho made at the beginning of each UI render
         // cycle typically makes the UPPER left corner (0,0).  glScissor seems 
         // to assume that (0,0) is always the LOWER left corner.
         viewport[3] - pRect->bottom,
         pRect->GetWidth(),
         pRect->GetHeight());
   }

   if (textLength < 0)
   {
      textLength = strlen(pszText);
   }

   int rowWidth = 0;
   int maxRowWidth = 0;
   int rowCount = 1;

   int x = pRect->left;
   int y = pRect->top;

   uint glyphFirst = kASCIIGlyphFirst;

   if (IsBitFlagSet(flags, kRT_Center))
   {
      // Calculate the string width
      for (int i = 0; i < textLength; i++)
      {
         uint c = pszText[i];

         if (c == '\n')
         {
            if (!(flags & kRT_SingleLine))
            {
               rowWidth = 0;
            }
            continue;
         }
         else if (c < glyphFirst)
         {
            continue;
         }

         uint index = c - glyphFirst;
         float tx1 = m_pGlyphs[index].texCoords[0];
         float tx2 = m_pGlyphs[index].texCoords[2];

         rowWidth += FloatToInt((tx2 - tx1) * m_widthMult);

         if (rowWidth > maxRowWidth)
         {
            maxRowWidth = rowWidth;
         }
      }

      x = (pRect->left + pRect->right - maxRowWidth) / 2;
   }

   if ((flags & (kRT_VCenter | kRT_SingleLine)) == (kRT_VCenter | kRT_SingleLine))
   {
      y = (pRect->top + pRect->bottom - m_rowHeight) / 2;
   }
   else if ((flags & (kRT_Bottom | kRT_SingleLine)) == (kRT_Bottom | kRT_SingleLine))
   {
      y = pRect->bottom - m_rowHeight;
   }

   struct sTextVertex
   {
      float u, v;
      float x, y, z;
   };

   sTextVertex * vertices = reinterpret_cast<sTextVertex *>(alloca(6 * textLength * sizeof(sTextVertex)));
   uint nVertices = 0;

   for (int i = 0; i < textLength; i++)
   {
      uint c = pszText[i];

      if (c == '\n')
      {
         if (!IsBitFlagSet(flags, kRT_SingleLine))
         {
            rowCount++;
            rowWidth = 0;
            x = pRect->left;
            y += m_rowHeight;
         }
         continue;
      }
      else if (c < glyphFirst)
      {
         continue;
      }

      uint index = c - glyphFirst;
      float tx1 = m_pGlyphs[index].texCoords[0];
      float ty1 = m_pGlyphs[index].texCoords[1];
      float tx2 = m_pGlyphs[index].texCoords[2];
      float ty2 = m_pGlyphs[index].texCoords[3];

      int w = FloatToInt((tx2 - tx1) * m_widthMult);
      int h = FloatToInt((ty2 - ty1) * m_heightMult);

      if (!IsBitFlagSet(flags, kRT_CalcRect))
      {
         if (c != ' ')
         {
            // TODO build a triangle strip instead of just a list of triangles

            sTextVertex * pVertex = &vertices[nVertices];
            float xPlusW = static_cast<float>(x + w);
            float yPlusH = static_cast<float>(y + h);

            // First Triangle

            // bottom left
            pVertex->u = tx1;
            pVertex->v = ty2;
            pVertex->x = static_cast<float>(x);
            pVertex->y = yPlusH;
            pVertex->z = 0;
            pVertex++;
            nVertices++;

            // bottom right
            pVertex->u = tx2;
            pVertex->v = ty2;
            pVertex->x = xPlusW;
            pVertex->y = yPlusH;
            pVertex->z = 0;
            pVertex++;
            nVertices++;

            // top right
            pVertex->u = tx2;
            pVertex->v = ty1;
            pVertex->x = xPlusW;
            pVertex->y = static_cast<float>(y);
            pVertex->z = 0;
            pVertex++;
            nVertices++;

            // Second Triangle

            // top right
            pVertex->u = tx2;
            pVertex->v = ty1;
            pVertex->x = xPlusW;
            pVertex->y = static_cast<float>(y);
            pVertex->z = 0;
            pVertex++;
            nVertices++;

            // top left
            pVertex->u = tx1;
            pVertex->v = ty1;
            pVertex->x = static_cast<float>(x);
            pVertex->y = static_cast<float>(y);
            pVertex->z = 0;
            pVertex++;
            nVertices++;

            // bottom left
            pVertex->u = tx1;
            pVertex->v = ty2;
            pVertex->x = static_cast<float>(x);
            pVertex->y = yPlusH;
            pVertex->z = 0;
            pVertex++;
            nVertices++;
         }
      }

      x += w;

      rowWidth += w;
      if (rowWidth > maxRowWidth)
      {
         maxRowWidth = rowWidth;
      }
   }

   if (!IsBitFlagSet(flags, kRT_CalcRect))
   {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, 0.0625f);

      if (!IsBitFlagSet(flags, kRT_NoBlend))
      {
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }

      glInterleavedArrays(GL_T2F_V3F, 0, vertices);

      if (IsBitFlagSet(flags, kRT_DropShadow))
      {
         glEnable(GL_STENCIL_TEST);
         glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
         glStencilFunc(GL_ALWAYS, kDropShadowStencilRef, kDropShadowStencilMask);
      }

      if (color != NULL)
      {
         glColor4fv(color);
      }

      glDrawArrays(GL_TRIANGLES, 0, nVertices);

      if (IsBitFlagSet(flags, kRT_DropShadow))
      {
         float modelview[16];
         glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();

         modelview[12] += m_dropShadowOffsetX;
         modelview[13] += m_dropShadowOffsetY;

         glLoadMatrixf(modelview);

         glStencilFunc(GL_NOTEQUAL, kDropShadowStencilRef, kDropShadowStencilMask);

         glColor4fv(m_dropShadowColor.GetPointer());
         glDrawArrays(GL_TRIANGLES, 0, nVertices);

         glPopMatrix();
      }
   }

   glPopAttrib();

   if (IsBitFlagSet(flags, kRT_CalcRect))
   {
      pRect->right = pRect->left + maxRowWidth;
      pRect->bottom = pRect->top + (rowCount * m_rowHeight);
   }

   return S_OK;
}

///////////////////////////////////////

tResult cTextureRenderFontW32::SetDropShadowState(float offsetX, float offsetY, const cColor & color)
{
   m_dropShadowOffsetX = offsetX;
   m_dropShadowOffsetY = offsetY;
   m_dropShadowColor = color;
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult RenderFontCreateGL(const tChar * pszFont, int pointSize, uint flags, IRenderFont * * ppFont)
{
   if (pszFont == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   if (pointSize < 0)
   {
      return E_INVALIDARG;;
   }

   cAutoIPtr<cTextureRenderFontW32> pFont(new cTextureRenderFontW32);
   if (!pFont)
   {
      return E_OUTOFMEMORY;
   }

   if (pFont->Create(pszFont, pointSize,
      (flags & kRFF_Bold) == kRFF_Bold,
      (flags & kRFF_Italic) == kRFF_Italic))
   {
      *ppFont = CTAddRef(static_cast<IRenderFont*>(pFont));
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
