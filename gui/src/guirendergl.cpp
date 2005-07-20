///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guirender.h"

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderDeviceGL
//

////////////////////////////////////////

tResult GUIRenderDeviceCreateGL(IGUIRenderDeviceContext * * ppRenderDevice)
{
   if (ppRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cGUIRenderDeviceGL> p(new cGUIRenderDeviceGL);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppRenderDevice = static_cast<IGUIRenderDeviceContext*>(CTAddRef(p));
   return S_OK;
}

////////////////////////////////////////

cGUIRenderDeviceGL::cGUIRenderDeviceGL()
 : m_scissorRectStackDepth(0)
{
}

////////////////////////////////////////

cGUIRenderDeviceGL::~cGUIRenderDeviceGL()
{
}

////////////////////////////////////////

void cGUIRenderDeviceGL::PushScissorRect(const tGUIRect & rect)
{
   glPushAttrib(GL_SCISSOR_BIT);

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

void cGUIRenderDeviceGL::PopScissorRect()
{
   Assert(m_scissorRectStackDepth > 0);
   m_scissorRectStackDepth--;

   glPopAttrib();
}

////////////////////////////////////////

void cGUIRenderDeviceGL::RenderSolidRect(const tGUIRect & rect, const tGUIColor & color)
{
   glBegin(GL_TRIANGLES);
      glColor4fv(color.GetPointer());

      glVertex2i(rect.left, rect.top);
      glVertex2i(rect.left, rect.bottom);
      glVertex2i(rect.right, rect.bottom);

      glVertex2i(rect.right, rect.bottom);
      glVertex2i(rect.right, rect.top);
      glVertex2i(rect.left, rect.top);
   glEnd();
}

////////////////////////////////////////

void cGUIRenderDeviceGL::RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                           const tGUIColor & bottomRight, const tGUIColor & face)
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

      glColor4fv(topLeft.GetPointer());

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

      glColor4fv(bottomRight.GetPointer());

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

      glColor4fv(face.GetPointer());

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

////////////////////////////////////////

void cGUIRenderDeviceGL::FlushQueue()
{
}

////////////////////////////////////////

void cGUIRenderDeviceGL::Begin2D()
{
   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);

   glGetIntegerv(GL_VIEWPORT, m_viewport);

   GLdouble width = static_cast<GLdouble>(m_viewport[2]);
   GLdouble height = static_cast<GLdouble>(m_viewport[3]);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, width, height, 0, -99999, 99999);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

////////////////////////////////////////

void cGUIRenderDeviceGL::End2D()
{
   glPopAttrib();
}

////////////////////////////////////////

tResult cGUIRenderDeviceGL::GetViewportSize(uint * pWidth, uint * pHeight)
{
   if (pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   *pWidth = viewport[2];
   *pHeight = viewport[3];

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
