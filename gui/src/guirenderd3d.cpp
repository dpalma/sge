///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guirenderd3d.h"

#if HAVE_DIRECTX
#include <d3d9.h>
#include <d3dx9.h>
#endif

#include "dbgalloc.h" // must be last header


#if HAVE_DIRECTX

////////////////////////////////////////////////////////////////////////////////

struct sGUIVertexD3D
{
   float x, y, z;
   uint32 color;
};

static const uint kGUIVertexFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderDeviceD3D
//

////////////////////////////////////////

tResult GUIRenderDeviceCreateD3D(IDirect3DDevice9 * pD3dDevice, IGUIRenderDeviceContext * * ppRenderDevice)
{
   if (pD3dDevice == NULL || ppRenderDevice == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<cGUIRenderDeviceD3D> p(new cGUIRenderDeviceD3D(pD3dDevice));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }

   *ppRenderDevice = static_cast<IGUIRenderDeviceContext*>(CTAddRef(p));
   return S_OK;
}

////////////////////////////////////////

cGUIRenderDeviceD3D::cGUIRenderDeviceD3D(IDirect3DDevice9 * pD3dDevice)
 : m_pD3dDevice(CTAddRef(pD3dDevice))
{
}

////////////////////////////////////////

cGUIRenderDeviceD3D::~cGUIRenderDeviceD3D()
{
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::PushScissorRect(const tGUIRect & rect)
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

void cGUIRenderDeviceD3D::PopScissorRect()
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

void cGUIRenderDeviceD3D::RenderSolidRect(const tGUIRect & rect, const tGUIColor & color)
{
#define VERT(x,y) \
   { static_cast<float>(x), static_cast<float>(y), 0, color.ToARGB8888() }

   sGUIVertexD3D verts[] =
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

void cGUIRenderDeviceD3D::RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                            const tGUIColor & bottomRight, const tGUIColor & face)
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

      sGUIVertexD3D verts[] =
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

////////////////////////////////////////

void cGUIRenderDeviceD3D::FlushQueue()
{
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::Begin2D()
{
   if (!m_pStateBlock)
   {
      m_pD3dDevice->BeginStateBlock();

      m_pD3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
      m_pD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
      m_pD3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

      D3DVIEWPORT9 viewport;
      m_pD3dDevice->GetViewport(&viewport);

      D3DXMATRIX ortho;
      D3DXMatrixOrthoOffCenterLH(&ortho,
         0, static_cast<float>(viewport.Width),
         static_cast<float>(viewport.Height), 0,
         viewport.MinZ, viewport.MaxZ);
      m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &ortho);

      m_pD3dDevice->SetFVF(kGUIVertexFVF);

      m_pD3dDevice->EndStateBlock(&m_pStateBlock);
   }

   if (!!m_pStateBlock)
   {
      m_pStateBlock->Apply();
   }
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::End2D()
{
}

////////////////////////////////////////

tResult cGUIRenderDeviceD3D::GetViewportSize(uint * pWidth, uint * pHeight)
{
   if (pWidth == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }

   D3DVIEWPORT9 viewport;
   m_pD3dDevice->GetViewport(&viewport);

   *pWidth = viewport.Width;
   *pHeight = viewport.Height;

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_DIRECTX
