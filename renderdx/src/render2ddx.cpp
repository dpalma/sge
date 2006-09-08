///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "render2ddx.h"

#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3dx9.h>

#include "dbgalloc.h" // must be last header


const uint kVertexFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRender2DDX
//

////////////////////////////////////////

tResult Render2DCreateDX(IDirect3DDevice9 * pD3dDevice, IRender2D * * ppRender2D)
{
   if (pD3dDevice == NULL || ppRender2D == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cRender2DDX> p(new cRender2DDX(pD3dDevice));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppRender2D = static_cast<IRender2D*>(CTAddRef(p));
   return S_OK;
}

////////////////////////////////////////

cRender2DDX::cRender2DDX(IDirect3DDevice9 * pD3dDevice)
 : m_pD3dDevice(CTAddRef(pD3dDevice))
{
}

////////////////////////////////////////

cRender2DDX::~cRender2DDX()
{
}

////////////////////////////////////////

tResult cRender2DDX::GetViewportSize(int * pWidth, int * pHeight) const
{
   if (pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }
   D3DVIEWPORT9 viewport;
   if (const_cast<IDirect3DDevice9 *>((const IDirect3DDevice9 *)m_pD3dDevice)->GetViewport(&viewport) == D3D_OK)
   {
      *pWidth = viewport.Width;
      *pHeight = viewport.Height;
      return S_OK;
   }
   return E_FAIL;
}

////////////////////////////////////////

void cRender2DDX::PushScissorRect(const tRect & rect)
{
   m_pD3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
   RECT scissorRect;
   scissorRect.left = rect.left;
   scissorRect.top = rect.top;
   scissorRect.right = rect.right;
   scissorRect.bottom = rect.bottom;
   m_pD3dDevice->SetScissorRect(&scissorRect);
   m_scissorRectStack.push(rect);
}

////////////////////////////////////////

void cRender2DDX::PopScissorRect()
{
   Assert(m_scissorRectStack.size() > 0);
   m_scissorRectStack.pop();
   if (m_scissorRectStack.empty())
   {
      m_pD3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
   }
   else
   {
      RECT scissorRect;
      scissorRect.left = m_scissorRectStack.top().left;
      scissorRect.top = m_scissorRectStack.top().top;
      scissorRect.right = m_scissorRectStack.top().right;
      scissorRect.bottom = m_scissorRectStack.top().bottom;
      m_pD3dDevice->SetScissorRect(&scissorRect);
   }
}

////////////////////////////////////////

void cRender2DDX::RenderSolidRect(const tRect & rect, const cColor & color)
{
#define VERT(x,y) \
   { static_cast<float>(x), static_cast<float>(y), 0, color.ToARGB8888() }

   sVertexD3D verts[] =
   {
      VERT(rect.left, rect.top),
      VERT(rect.left, rect.bottom),
      VERT(rect.right, rect.bottom),
      VERT(rect.right, rect.bottom),
      VERT(rect.right, rect.top),
      VERT(rect.left, rect.top),
   };

#undef VERT

   Verify(m_pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, _countof(verts) / 3, verts, sizeof(verts[0])) == D3D_OK);
}

////////////////////////////////////////

void cRender2DDX::RenderBeveledRect(const tRect & rect, int bevel, const cColor & topLeft,
                                    const cColor & bottomRight, const cColor & face)
{
   if (bevel == 0)
   {
      RenderSolidRect(rect, face);
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

#define VERT(x,y,c) \
   { static_cast<float>(x), static_cast<float>(y), 0, c.ToARGB8888() }

      sVertexD3D verts[] =
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

#undef VERT

      Verify(m_pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, _countof(verts) / 3, verts, sizeof(verts[0])) == D3D_OK);
   }
}

///////////////////////////////////////////////////////////////////////////////
