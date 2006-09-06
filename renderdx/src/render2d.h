///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDER2D_H
#define INCLUDED_RENDER2D_H

#include "renderapi.h"

#include <stack>

#ifdef _MSC_VER
#pragma once
#endif

#if HAVE_DIRECTX

F_DECLARE_INTERFACE(IDirect3DDevice9);
F_DECLARE_INTERFACE(IDirect3DStateBlock9);


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRender2DDX
//

class cRender2DDX : public cComObject<IMPLEMENTS(IRender2D)>
{
   cRender2DDX(const cRender2DDX &);
   void operator =(const cRender2DDX &);

public:
   cRender2DDX(IDirect3DDevice9 * pD3dDevice);
   ~cRender2DDX();

   virtual void PushScissorRect(const tRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tRect & rect, const cColor & color);
   virtual void RenderBeveledRect(const tRect & rect, int bevel, const cColor & topLeft,
                                  const cColor & bottomRight, const cColor & face);

   void Begin2D(int width, int height);
   void End2D();

private:
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
   typedef std::stack<tRect> tRectStack;
   tRectStack m_scissorRectStack;
   int m_viewport[4];
   cAutoIPtr<IDirect3DStateBlock9> m_pStateBlock;
};

#endif // HAVE_DIRECTX

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDER2D_H
