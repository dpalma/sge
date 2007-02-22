///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render2d.h"

#include "tech/globalobj.h"

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

struct s2DVertex
{
   float x, y, z;
   float r, g, b, a;
};

static const sVertexElement g_2dVertexDecl[] =
{
   { kVEU_Position, kVET_Float3, 0, 0 },
   { kVEU_Color,    kVET_Float4, 0, 3 * sizeof(float) },
};

void cRender2DGL::RenderBeveledRect(const tRect & rect, int bevel,
                                    const float topLeft[4],
                                    const float bottomRight[4],
                                    const float face[4])
{
   if (bevel == 0)
   {
      RenderSolidRect(rect, face);
   }
   else
   {
      float x0 = static_cast<float>(rect.left);
      float x1 = static_cast<float>(rect.left + bevel);
      float x2 = static_cast<float>(rect.right - bevel);
      float x3 = static_cast<float>(rect.right);

      float y0 = static_cast<float>(rect.top);
      float y1 = static_cast<float>(rect.top + bevel);
      float y2 = static_cast<float>(rect.bottom - bevel);
      float y3 = static_cast<float>(rect.bottom);

#define VERT(x,y,c) { x, y, 0, (c)[0], (c)[1], (c)[2], (c)[3] }

      s2DVertex verts[] =
      {
         VERT(x0, y0, topLeft),
         VERT(x0, y3, topLeft),
         VERT(x1, y2, topLeft),

         VERT(x0, y0, topLeft),
         VERT(x1, y2, topLeft),
         VERT(x1, y1, topLeft),

         VERT(x0, y0, topLeft),
         VERT(x2, y1, topLeft),
         VERT(x3, y0, topLeft),

         VERT(x0, y0, topLeft),
         VERT(x1, y1, topLeft),
         VERT(x2, y1, topLeft),

         VERT(x0, y3, bottomRight),
         VERT(x3, y3, bottomRight),
         VERT(x1, y2, bottomRight),

         VERT(x1, y2, bottomRight),
         VERT(x3, y3, bottomRight),
         VERT(x2, y2, bottomRight),

         VERT(x3, y0, bottomRight),
         VERT(x2, y1, bottomRight),
         VERT(x3, y3, bottomRight),

         VERT(x2, y1, bottomRight),
         VERT(x2, y2, bottomRight),
         VERT(x3, y3, bottomRight),

         VERT(x1, y1, face),
         VERT(x2, y2, face),
         VERT(x2, y1, face),

         VERT(x2, y2, face),
         VERT(x1, y1, face),
         VERT(x1, y2, face),
      };

      UseGlobal(Renderer);
      pRenderer->SetVertexFormat(g_2dVertexDecl, _countof(g_2dVertexDecl));
      pRenderer->SubmitVertices(verts, _countof(verts));
      pRenderer->Render(kPT_Triangles, 0, _countof(verts));
   }
}

///////////////////////////////////////////////////////////////////////////////
