///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "d3dguirender.h"

#include <d3d9.h>

#include "dbgalloc.h" // must be last header


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
   // TODO
   //glBegin(GL_TRIANGLES);
   //   glColor4fv(color.GetPointer());

   //   glVertex2i(rect.left, rect.top);
   //   glVertex2i(rect.left, rect.bottom);
   //   glVertex2i(rect.right, rect.bottom);

   //   glVertex2i(rect.right, rect.bottom);
   //   glVertex2i(rect.right, rect.top);
   //   glVertex2i(rect.left, rect.top);
   //glEnd();
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
   // TODO
   //glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

   //glDisable(GL_DEPTH_TEST);
   //glDisable(GL_LIGHTING);
   //glDisable(GL_CULL_FACE);

   //glGetIntegerv(GL_VIEWPORT, m_viewport);

   //GLdouble width = static_cast<GLdouble>(m_viewport[2]);
   //GLdouble height = static_cast<GLdouble>(m_viewport[3]);

   //glMatrixMode(GL_PROJECTION);
   //glLoadIdentity();
   //glOrtho(0, width, height, 0, -99999, 99999);

   //glMatrixMode(GL_MODELVIEW);
   //glLoadIdentity();
}

////////////////////////////////////////

void cGUIRenderDeviceD3D::End2D()
{
   // TODO
   //glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
