///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "techmath.h"
#include "ggl.h"
#include "font.h"
#include "uirender.h"
#include "render.h"
#include "matrix4.h"

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

cUISize UIGetRootContainerSize()
{
   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   return cUISize(viewport[2], viewport[3]);
}

///////////////////////////////////////////////////////////////////////////////

void UIPushClipRect(const cUIRect & rect)
{
   glPushAttrib(GL_SCISSOR_BIT);

   glEnable(GL_SCISSOR_TEST);
   glScissor(
      Round(rect.left),
      // @HACK: the call to glOrtho made at the beginning of each UI render
      // cycle makes the UPPER left corner (0,0).  glScissor seems to assume
      // that (0,0) is always the LOWER left corner.
      Round(UIGetRootContainerSize().height - rect.bottom),
      Round(rect.GetWidth()),
      Round(rect.GetHeight()));
}

void UIPopClipRect()
{
   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

cUISize UIMeasureText(const char * pszText, int textLen, IFont * pFont /*=NULL*/)
{
   Assert(pszText != NULL);

   if (textLen < 0)
      textLen = strlen(pszText);

   float textWidth = 0, textHeight = 0;

   if (pFont == NULL)
   {
      cAutoIPtr<IFont> pFont2 = FontCreateDefault();
      pFont2->MeasureText(pszText, textLen, &textWidth, &textHeight);
   }
   else
   {
      pFont->MeasureText(pszText, textLen, &textWidth, &textHeight);
   }

   return cUISize(textWidth, textHeight);
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawText(const char * pszText, int textLen,
                const cUIRect * pRect, uint flags /*=kTextDefault*/,
                IFont * pFont /*=NULL*/,
                const cUIColor & color /*=cUIColor(1,1,1,1)*/)
{
   Assert(pszText != NULL);
   Assert(pRect != NULL);
   Assert(pFont != NULL);

   if (pszText == NULL
      || pRect == NULL
      || pFont == NULL)
      return;

   float textWidth, textHeight;
   pFont->MeasureText(pszText, textLen, &textWidth, &textHeight);

   float textX =  pRect->left;
   if (flags & kTextCenter)
      textX += (pRect->GetWidth() - textWidth) * 0.5;
   float textY = pRect->top;
   if (flags & kTextVCenter)
      textY += (pRect->GetHeight() - textHeight) * 0.5;

   glPushAttrib(GL_SCISSOR_BIT | GL_CURRENT_BIT);

   if (!(flags & kTextNoClip))
   {
      glEnable(GL_SCISSOR_TEST);
      glScissor(
         Round(pRect->left),
         Round(UIGetRootContainerSize().height - pRect->bottom), //@HACK
         Round(pRect->GetWidth()),
         Round(pRect->GetHeight()));
   }

   glColor4fv(color.GetPointer());

   pFont->DrawText(textX, textY, pszText, textLen);

   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawText(const cUIPoint & point,
                const char * pszText, int textLen /*=-1*/)
{
   cAutoIPtr<IFont> pFont = FontCreateDefault();
   if (pFont)
      pFont->DrawText(point.x, point.y, pszText, textLen);
}

///////////////////////////////////////////////////////////////////////////////

void UIDraw3dRect(const cUIRect & rect,
                  int bevel,
                  const cUIColor & topLeft,
                  const cUIColor & bottomRight,
                  const cUIColor & face)
{
   float x0 = rect.left;
   float x1 = rect.left + bevel;
   float x2 = rect.right - bevel;
   float x3 = rect.right;

   float y0 = rect.top;
   float y1 = rect.top + bevel;
   float y2 = rect.bottom - bevel;
   float y3 = rect.bottom;

   glBegin(GL_TRIANGLES);

      glColor4fv(topLeft.GetPointer());

      glVertex2f(x0, y0);
      glVertex2f(x0, y3);
      glVertex2f(x1, y2);

      glVertex2f(x0, y0);
      glVertex2f(x1, y2);
      glVertex2f(x1, y1);

      glVertex2f(x0, y0);
      glVertex2f(x2, y1);
      glVertex2f(x3, y0);

      glVertex2f(x0, y0);
      glVertex2f(x1, y1);
      glVertex2f(x2, y1);

      glColor4fv(bottomRight.GetPointer());

      glVertex2f(x0, y3);
      glVertex2f(x3, y3);
      glVertex2f(x1, y2);

      glVertex2f(x1, y2);
      glVertex2f(x3, y3);
      glVertex2f(x2, y2);

      glVertex2f(x3, y0);
      glVertex2f(x2, y1);
      glVertex2f(x3, y3);

      glVertex2f(x2, y1);
      glVertex2f(x2, y2);
      glVertex2f(x3, y3);

      glColor4fv(face.GetPointer());

      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
      glVertex2f(x2, y1);

      glVertex2f(x2, y2);
      glVertex2f(x1, y1);
      glVertex2f(x1, y2);

   glEnd();
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawGradientRect(const cUIRect & rect,
                        const cUIColor & topLeft,
                        const cUIColor & topRight,
                        const cUIColor & bottomRight,
                        const cUIColor & bottomLeft)
{
   glPushAttrib(GL_CURRENT_BIT);
   glBegin(GL_QUADS);
      glColor4fv(topLeft.GetPointer());
      glVertex2f(rect.left, rect.top);
      glColor4fv(bottomLeft.GetPointer());
      glVertex2f(rect.left, rect.bottom);
      glColor4fv(bottomRight.GetPointer());
      glVertex2f(rect.right, rect.bottom);
      glColor4fv(topRight.GetPointer());
      glVertex2f(rect.right, rect.top);
   glEnd();
   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawSolidRect(const cUIRect & rect,
                     const cUIColor & color)
{
   UIDrawGradientRect(rect, color, color, color, color);
}

///////////////////////////////////////////////////////////////////////////////

void UIDrawTextureRect(const cUIRect & rect,
                       ITexture * pTexture)
{
   uint texId;
   if (pTexture != NULL && pTexture->GetTextureId(&texId) == S_OK)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_ALPHA_TEST);
      glBindTexture(GL_TEXTURE_2D, texId);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glColor4f(1,1,1,1); // set color and alpha multipliers to one
      glBegin(GL_QUADS);
         glTexCoord2i(0, 1);
         glVertex2f(rect.left, rect.top);
         glTexCoord2i(0, 0);
         glVertex2f(rect.left, rect.bottom);
         glTexCoord2i(1, 0);
         glVertex2f(rect.right, rect.bottom);
         glTexCoord2i(1, 1);
         glVertex2f(rect.right, rect.top);
      glEnd();
      glPopAttrib();
   }
}

///////////////////////////////////////////////////////////////////////////////
