///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDER2DDX_H
#define INCLUDED_RENDER2DDX_H

#include "render/renderapi.h"

#include <stack>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IDirect3DDevice9);
F_DECLARE_INTERFACE(IDirect3DStateBlock9);


////////////////////////////////////////////////////////////////////////////////

struct sVertexD3D
{
   float x, y, z;
   uint32 color;
};

extern const uint kVertexFVF;


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRender2DDX
//

class cRender2DDX : public cComObject<IMPLEMENTS(IRender2D)>
{
   cRender2DDX(const cRender2DDX &);
   const cRender2DDX & operator =(const cRender2DDX &);

public:
   cRender2DDX(IDirect3DDevice9 * pD3dDevice);
   ~cRender2DDX();

   virtual tResult GetViewportSize(int * pWidth, int * pHeight) const;

   virtual void PushScissorRect(const tRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tRect & rect, const float color[4]);
   virtual void RenderBeveledRect(const tRect & rect, int bevel,
                                  const float topLeft[4],
                                  const float bottomRight[4],
                                  const float face[4]);

private:
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
   typedef std::stack<tRect> tRectStack;
   tRectStack m_scissorRectStack;
   int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDER2DDX_H
