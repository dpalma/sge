///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "d3dguirender.h"

#include <d3d9.h>
#include <d3dx9.h>

#undef CreateFont

#include "dbgalloc.h" // must be last header


extern tResult GUIFontCreateD3D(IDirect3DDevice9 * pD3dDevice,
                                const cGUIFontDesc & fontDesc,
                                IGUIFont * * ppFont);


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
   tFontMap::iterator iter = m_fontMap.begin();
   tFontMap::iterator end = m_fontMap.end();
   for (; iter != end; iter++)
   {
      SafeRelease(iter->second);
   }
   m_fontMap.clear();
}

////////////////////////////////////////

tResult cGUIRenderDeviceD3D::CreateFont(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont)
{
   if (ppFont == NULL)
   {
      return E_POINTER;
   }

   tFontMap::iterator f = m_fontMap.find(fontDesc);
   if (f != m_fontMap.end())
   {
      *ppFont = CTAddRef(f->second);
      return S_OK;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GUIFontCreateD3D(m_pD3dDevice, fontDesc, &pFont) == S_OK)
   {
      m_fontMap[fontDesc] = CTAddRef(pFont);
      *ppFont = CTAddRef(pFont);
      return S_OK;
   }

   return E_FAIL;
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
   sGUIVertexD3D verts[6];
   memset(verts, 0, sizeof(verts));

   for (int i = 0; i < _countof(verts); i++)
   {
      verts[i].color = color.GetARGB();
   }

   verts[0].x = rect.left;
   verts[0].y = rect.top;
   verts[1].x = rect.left;
   verts[1].y = rect.bottom;
   verts[2].x = rect.right;
   verts[2].y = rect.bottom;
   verts[3].x = rect.right;
   verts[3].y = rect.bottom;
   verts[4].x = rect.right;
   verts[4].y = rect.top;
   verts[5].x = rect.left;
   verts[5].y = rect.top;

   m_pD3dDevice->SetFVF(kGUIVertexFVF);
   m_pD3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, verts, sizeof(sGUIVertexD3D));
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                            const tGUIColor & bottomRight, const tGUIColor & face)
{
   // TODO
   //glPushAttrib(GL_ENABLE_BIT);
   //glDisable(GL_TEXTURE_2D);

   //if (bevel == 0)
   //{
   //   RenderSolidRect(rect, face);
   //}
   //else
   //{
   //   glBegin(GL_TRIANGLES);

   //   int x0 = rect.left;
   //   int x1 = rect.left + bevel;
   //   int x2 = rect.right - bevel;
   //   int x3 = rect.right;

   //   int y0 = rect.top;
   //   int y1 = rect.top + bevel;
   //   int y2 = rect.bottom - bevel;
   //   int y3 = rect.bottom;

   //   glColor4fv(topLeft.GetPointer());

   //   glVertex2i(x0, y0);
   //   glVertex2i(x0, y3);
   //   glVertex2i(x1, y2);

   //   glVertex2i(x0, y0);
   //   glVertex2i(x1, y2);
   //   glVertex2i(x1, y1);

   //   glVertex2i(x0, y0);
   //   glVertex2i(x2, y1);
   //   glVertex2i(x3, y0);

   //   glVertex2i(x0, y0);
   //   glVertex2i(x1, y1);
   //   glVertex2i(x2, y1);

   //   glColor4fv(bottomRight.GetPointer());

   //   glVertex2i(x0, y3);
   //   glVertex2i(x3, y3);
   //   glVertex2i(x1, y2);

   //   glVertex2i(x1, y2);
   //   glVertex2i(x3, y3);
   //   glVertex2i(x2, y2);

   //   glVertex2i(x3, y0);
   //   glVertex2i(x2, y1);
   //   glVertex2i(x3, y3);

   //   glVertex2i(x2, y1);
   //   glVertex2i(x2, y2);
   //   glVertex2i(x3, y3);

   //   glColor4fv(face.GetPointer());

   //   glVertex2i(x1, y1);
   //   glVertex2i(x2, y2);
   //   glVertex2i(x2, y1);

   //   glVertex2i(x2, y2);
   //   glVertex2i(x1, y1);
   //   glVertex2i(x1, y2);

   //   glEnd();
   //}

   //glPopAttrib();
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::FlushQueue()
{
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::Begin2D()
{
   m_pD3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
   m_pD3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

   D3DVIEWPORT9 viewport;
   m_pD3dDevice->GetViewport(&viewport);

   D3DXMATRIX ortho;
   D3DXMatrixOrthoRH(&ortho, viewport.Width, viewport.Height, viewport.MinZ, viewport.MaxZ);

   m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &ortho);
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
