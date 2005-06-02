///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guirender.h"

#include "font.h"
#include "color.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // HACK
#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderingTools
//

///////////////////////////////////////

cGUIRenderingTools::cGUIRenderingTools()
{
}

///////////////////////////////////////

cGUIRenderingTools::~cGUIRenderingTools()
{
}

///////////////////////////////////////

tResult cGUIRenderingTools::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::Term()
{
   SafeRelease(m_pFont);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::SetDefaultFont(IRenderFont * pFont)
{
   SafeRelease(m_pFont);
   m_pFont = CTAddRef(pFont);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIRenderingTools::GetDefaultFont(IRenderFont * * ppFont)
{
   if (!m_pFont)
   {
      if (FAILED(FontCreateDefault(&m_pFont)))
      {
         return E_FAIL;
      }
   }
   return m_pFont.GetPointer(ppFont);
}

///////////////////////////////////////

static uint32 PackColor(const tGUIColor & color)
{
   byte r = (byte)(color.GetRed() * 255);
   byte g = (byte)(color.GetGreen() * 255);
   byte b = (byte)(color.GetBlue() * 255);
   byte a = (byte)(color.GetAlpha() * 255);
   return ARGB(a,b,g,r);
}

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


///////////////////////////////////////

void GUIRenderingToolsCreate()
{
   cAutoIPtr<IGUIRenderingTools>(new cGUIRenderingTools);
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
