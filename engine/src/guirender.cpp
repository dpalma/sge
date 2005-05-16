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

void GlRender3dRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                    const tGUIColor & bottomRight, const tGUIColor & face)
{
#define FillVertex(index, x, y, clr) \
   do { \
      glColor4fv(clr.GetPointer()); \
      glVertex2i(x, y); \
   } while (0)

   if (bevel == 0)
   {
      FillVertex(0, rect.left, rect.top, face);
      FillVertex(1, rect.left, rect.bottom, face);
      FillVertex(2, rect.right, rect.bottom, face);

      FillVertex(3, rect.right, rect.bottom, face);
      FillVertex(4, rect.right, rect.top, face);
      FillVertex(5, rect.left, rect.top, face);
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

      FillVertex(0, x0, y0, topLeft);
      FillVertex(1, x0, y3, topLeft);
      FillVertex(2, x1, y2, topLeft);

      FillVertex(3, x0, y0, topLeft);
      FillVertex(4, x1, y2, topLeft);
      FillVertex(5, x1, y1, topLeft);

      FillVertex(6, x0, y0, topLeft);
      FillVertex(7, x2, y1, topLeft);
      FillVertex(8, x3, y0, topLeft);

      FillVertex(9, x0, y0, topLeft);
      FillVertex(10, x1, y1, topLeft);
      FillVertex(11, x2, y1, topLeft);

      FillVertex(12, x0, y3, bottomRight);
      FillVertex(13, x3, y3, bottomRight);
      FillVertex(14, x1, y2, bottomRight);

      FillVertex(15, x1, y2, bottomRight);
      FillVertex(16, x3, y3, bottomRight);
      FillVertex(17, x2, y2, bottomRight);

      FillVertex(18, x3, y0, bottomRight);
      FillVertex(19, x2, y1, bottomRight);
      FillVertex(20, x3, y3, bottomRight);

      FillVertex(21, x2, y1, bottomRight);
      FillVertex(22, x2, y2, bottomRight);
      FillVertex(23, x3, y3, bottomRight);

      FillVertex(24, x1, y1, face);
      FillVertex(25, x2, y2, face);
      FillVertex(26, x2, y1, face);

      FillVertex(27, x2, y2, face);
      FillVertex(28, x1, y1, face);
      FillVertex(29, x1, y2, face);
   }

#undef FillVertex
}


///////////////////////////////////////

void GUIRenderingToolsCreate()
{
   cAutoIPtr<IGUIRenderingTools>(new cGUIRenderingTools);
}

///////////////////////////////////////////////////////////////////////////////
