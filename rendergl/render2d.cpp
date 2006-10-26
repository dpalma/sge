///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render2d.h"

#include <GL/glew.h>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRender2DGL
//

////////////////////////////////////////

tResult Render2DCreateGL(IRender2D * * ppRender2D)
{
   if (ppRender2D == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cRender2DGL> p(new cRender2DGL);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppRender2D = static_cast<IRender2D*>(CTAddRef(p));
   return S_OK;
}

////////////////////////////////////////

cRender2DGL::cRender2DGL()
 : m_scissorRectStackDepth(0)
{
}

////////////////////////////////////////

cRender2DGL::~cRender2DGL()
{
}

////////////////////////////////////////

tResult cRender2DGL::GetViewportSize(int * pWidth, int * pHeight) const
{
   if (pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }

   glGetIntegerv(GL_VIEWPORT, m_viewport);

   *pWidth = m_viewport[2] - m_viewport[0];
   *pHeight = m_viewport[3] - m_viewport[1];
   return S_OK;
}

////////////////////////////////////////

void cRender2DGL::PushScissorRect(const tRect & rect)
{
   glPushAttrib(GL_SCISSOR_BIT);

   glGetIntegerv(GL_VIEWPORT, m_viewport);

   glEnable(GL_SCISSOR_TEST);
   glScissor(
      rect.left,
      // @HACK: the call to glOrtho made at the beginning of each UI render
      // cycle typically makes the UPPER left corner (0,0).  glScissor seems 
      // to assume that (0,0) is always the LOWER left corner.
      m_viewport[3] - rect.bottom,
      rect.GetWidth(),
      rect.GetHeight());

   m_scissorRectStackDepth++;
}

////////////////////////////////////////

void cRender2DGL::PopScissorRect()
{
   Assert(m_scissorRectStackDepth > 0);
   m_scissorRectStackDepth--;

   glPopAttrib();
}

////////////////////////////////////////

void cRender2DGL::RenderSolidRect(const tRect & rect, const float color[4])
{
   glBegin(GL_TRIANGLES);
      glColor4fv(color);

      glVertex2i(rect.left, rect.top);
      glVertex2i(rect.left, rect.bottom);
      glVertex2i(rect.right, rect.bottom);

      glVertex2i(rect.right, rect.bottom);
      glVertex2i(rect.right, rect.top);
      glVertex2i(rect.left, rect.top);
   glEnd();
}

////////////////////////////////////////

void cRender2DGL::RenderBeveledRect(const tRect & rect, int bevel,
                                    const float topLeft[4],
                                    const float bottomRight[4],
                                    const float face[4])
{
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_TEXTURE_2D);

   if (bevel == 0)
   {
      RenderSolidRect(rect, face);
   }
   else
   {
      glBegin(GL_TRIANGLES);

      int x0 = rect.left;
      int x1 = rect.left + bevel;
      int x2 = rect.right - bevel;
      int x3 = rect.right;

      int y0 = rect.top;
      int y1 = rect.top + bevel;
      int y2 = rect.bottom - bevel;
      int y3 = rect.bottom;

      glColor4fv(topLeft);

      glVertex2i(x0, y0);
      glVertex2i(x0, y3);
      glVertex2i(x1, y2);

      glVertex2i(x0, y0);
      glVertex2i(x1, y2);
      glVertex2i(x1, y1);

      glVertex2i(x0, y0);
      glVertex2i(x2, y1);
      glVertex2i(x3, y0);

      glVertex2i(x0, y0);
      glVertex2i(x1, y1);
      glVertex2i(x2, y1);

      glColor4fv(bottomRight);

      glVertex2i(x0, y3);
      glVertex2i(x3, y3);
      glVertex2i(x1, y2);

      glVertex2i(x1, y2);
      glVertex2i(x3, y3);
      glVertex2i(x2, y2);

      glVertex2i(x3, y0);
      glVertex2i(x2, y1);
      glVertex2i(x3, y3);

      glVertex2i(x2, y1);
      glVertex2i(x2, y2);
      glVertex2i(x3, y3);

      glColor4fv(face);

      glVertex2i(x1, y1);
      glVertex2i(x2, y2);
      glVertex2i(x2, y1);

      glVertex2i(x2, y2);
      glVertex2i(x1, y1);
      glVertex2i(x1, y2);

      glEnd();
   }

   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
