////////////////////////////////////////////////////////////////////////////////
// $Id: renderfontfreetype.cpp 1426 2006-08-15 19:09:05Z dpalma $

#include "stdhdr.h"

#include "renderfontftgl.h"

#include "sys.h"

#include "color.h"
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

   *ppFont = new cRenderFontFtgl(pFtglFont);
   if ((*ppFont) == NULL)
   {
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFtgl::MeasureText(const tChar * pszText, int /*textLength*/, int * pWidth, int * pHeight) const
{
   if (pszText == NULL || pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }

   if (m_pFont == NULL)
   {
      return E_FAIL;
   }

   float llx = 0, lly = 0, llz = 0, urx = 0, ury = 0, urz = 0;
   m_pFont->BBox(pszText, llx, lly, llz, urx, ury, urz);

   *pWidth = FloatToInt(urx - llx);
   *pHeight = FloatToInt(ury - lly);
   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFtgl::RenderText(const tChar * pszText, int /*textLength*/, int x, int y) const
{
   if (pszText == NULL)
   {
      return E_POINTER;
   }

   if (m_pFont == NULL)
   {
      return E_FAIL;
   }

   glPushMatrix();
   glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), 0);
   glScalef(1, -1, 1);
   glEnable(GL_TEXTURE_2D);
   m_pFont->Render(pszText);
   glPopMatrix();

   return S_OK;
}

////////////////////////////////////////

tResult cRenderFontFtgl::RenderText(const tChar * pszText, int textLength, tRect * pRect, uint flags, const cColor & color) const
{
   if (pszText == NULL || pRect == NULL)
   {
      return E_POINTER;
   }

   if (m_pFont == NULL)
   {
      return E_FAIL;
   }

   WarnMsgIf(IsFlagSet(flags, kRT_NoBlend), "NoBlend flag not supported\n");

   if (IsFlagSet(flags, kRT_CalcRect))
   {
      float llx = 0, lly = 0, llz = 0, urx = 0, ury = 0, urz = 0;
      m_pFont->BBox(pszText, llx, lly, llz, urx, ury, urz);
      pRect->right = pRect->left + FloatToInt(urx - llx);
      pRect->bottom = pRect->top + FloatToInt(ury - lly);
   }
   else
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

      glColor4fv(color.GetPointer());

      glPushMatrix();
      glTranslatef(static_cast<GLfloat>(pRect->left), static_cast<GLfloat>(pRect->bottom), 0);
      glScalef(1, -1, 1);
      m_pFont->Render(pszText);
      glPopMatrix();

      glPopAttrib();
   }

   return S_OK;
}

////////////////////////////////////////

tResult RenderFontCreateFTGL(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont)
{
   return cRenderFontFtgl::Create(pszFont, fontPointSize, ppFont);
}

////////////////////////////////////////////////////////////////////////////////
