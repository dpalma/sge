////////////////////////////////////////////////////////////////////////////////
// $Id: renderfontfreetype.cpp 1426 2006-08-15 19:09:05Z dpalma $

#include "stdhdr.h"

#include "renderfontftgl.h"

#include "sys.h"

#include "filepath.h"
#include "filespec.h"

#include <GL/glew.h>
#include <FTGLTextureFont.h>

#include <cstring>

#include "dbgalloc.h" // must be last header

#define IsFlagSet(var, bit) (((var) & (bit)) == (bit))


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderFontFtgl
//

////////////////////////////////////////

cRenderFontFtgl::cRenderFontFtgl(FTGLTextureFont * pFtglFont)
 : m_pFont(pFtglFont)
{
}

////////////////////////////////////////

cRenderFontFtgl::~cRenderFontFtgl()
{
   delete m_pFont, m_pFont = NULL;
}

////////////////////////////////////////

tResult cRenderFontFtgl::Create(const tChar * pszFont, int fontPointSize, IRenderFont * * ppFont)
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

   if (_tcslen(pszFont) == 0)
   {
      return E_INVALIDARG;
   }

   cFileSpec fontName(pszFont);
   fontName.SetFileExt(_T("ttf"));

   cFilePath fontPath;
   if (SysGetFontPath(&fontPath) == S_OK)
   {
      fontName.SetPath(fontPath);
   }

   FTGLTextureFont * pFtglFont = new FTGLTextureFont(fontName.CStr());
   if (pFtglFont == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (pFtglFont->Error() != FT_Err_Ok
      || !pFtglFont->FaceSize(fontPointSize))
   {
      delete pFtglFont, pFtglFont = NULL;
      return E_FAIL;
   }

   pFtglFont->CharMap(FT_ENCODING_UNICODE);

   *ppFont = new cRenderFontFtgl(pFtglFont);
   if ((*ppFont) == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFtgl::RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const float color[4]) const
{
   if (pszText == NULL || pRect == NULL)
   {
      return E_POINTER;
   }

   if (m_pFont == NULL)
   {
      return E_FAIL;
   }

   if (textLength < 0)
   {
      textLength = _tcslen(pszText);
   }

   WarnMsgIf(IsFlagSet(flags, kRT_NoBlend), "NoBlend flag not supported\n");

   if (!IsFlagSet(flags, kRT_CalcRect))
   {
      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

      glEnable(GL_TEXTURE_2D);

      if (!IsFlagSet(flags, kRT_NoClip))
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

      if (color != NULL)
      {
         glColor4fv(color);
      }
   }

   static const tChar szLineBreakChars[] = _T("\r\n");

   static const int kStringLengthSanityCheck = 2048;

   tResult result = S_OK;

   float y = static_cast<float>(pRect->top);
   float boxWidth = 0, boxHeight = 0;

   float lineHeight = m_pFont->LineHeight();
   float descender = m_pFont->Descender();

   int accumLength = 0, breakLength = _tcscspn(pszText, szLineBreakChars);
   while ((*pszText != 0) && (accumLength < textLength))
   {
      if (breakLength >= kStringLengthSanityCheck)
      {
         WarnMsg1("Insane string segment length: %d\n", breakLength);
         result = E_FAIL;
         break;
      }

      tChar * pszLine = static_cast<tChar *>(alloca(breakLength + 1));
      memcpy(pszLine, pszText, breakLength * sizeof(tChar));
      pszLine[breakLength] = 0;

      float llx = 0, lly = 0, llz = 0, urx = 0, ury = 0, urz = 0;
      m_pFont->BBox(pszLine, llx, lly, llz, urx, ury, urz);
      float lineWidth = (urx - llx);

      if (IsFlagSet(flags, kRT_CalcRect))
      {
         if (lineWidth > boxWidth)
         {
            boxWidth = lineWidth;
         }
         boxHeight += lineHeight;
      }
      else
      {
         float x = static_cast<GLfloat>(pRect->left);

         if (IsFlagSet(flags, kRT_Center))
         {
            x += ((pRect->right - pRect->left - lineWidth) * 0.5f);
         }

         glPushMatrix();
         glTranslatef(x, y + lineHeight + descender, 0);
         glScalef(1, -1, 1);
         m_pFont->Render(pszLine);
         glPopMatrix();
      }

      accumLength += breakLength;

      pszText += breakLength;
      if (*pszText == _T('\n'))
      {
         y += m_pFont->LineHeight();
      }
      while (_tcschr(szLineBreakChars, *pszText) != NULL)
      {
         pszText++;
         accumLength++;
      }
      breakLength = _tcscspn(pszText, szLineBreakChars);
   }

   if (!IsFlagSet(flags, kRT_CalcRect))
   {
      glPopAttrib();
   }
   else
   {
      pRect->right = pRect->left + FloatToInt(boxWidth);
      pRect->bottom = pRect->top + FloatToInt(boxHeight);
   }

   return result;
}

////////////////////////////////////////

tResult RenderFontCreateFTGL(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont)
{
   return cRenderFontFtgl::Create(pszFont, fontPointSize, ppFont);
}

////////////////////////////////////////////////////////////////////////////////
