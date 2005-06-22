///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiapi.h"

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void GlRenderBevelledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                          const tGUIColor & bottomRight, const tGUIColor & face)
{
   glPushAttrib(GL_ENABLE_BIT);
   glDisable(GL_TEXTURE_2D);

   glBegin(GL_TRIANGLES);

   if (bevel == 0)
   {
      glColor4fv(face.GetPointer());

      glVertex2i(rect.left, rect.top);
      glVertex2i(rect.left, rect.bottom);
      glVertex2i(rect.right, rect.bottom);

      glVertex2i(rect.right, rect.bottom);
      glVertex2i(rect.right, rect.top);
      glVertex2i(rect.left, rect.top);

   }
   else
   {
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
   }

   glEnd();
   glPopAttrib();
}


///////////////////////////////////////////////////////////////////////////////

void GlBegin2D()
{
   glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);

   GLint viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   GLdouble width = static_cast<GLdouble>(viewport[2]);
   GLdouble height = static_cast<GLdouble>(viewport[3]);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, width, height, 0, -99999, 99999);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void GlEnd2D()
{
   glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
