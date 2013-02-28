///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERTARGETDX_H
#define INCLUDED_RENDERTARGETDX_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IDirect3DDevice9);

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetDX
//

class cRenderTargetDX : public cComObject<IMPLEMENTS(IRenderTarget)>
{
   cRenderTargetDX(const cRenderTargetDX &);
   const cRenderTargetDX & operator =(const cRenderTargetDX &);

   friend tResult RenderTargetDXCreate(IDirect3DDevice9 * pD3dDevice, IRenderTarget * * ppRenderTarget);

public:
   cRenderTargetDX(IDirect3DDevice9 * pD3dDevice);
   ~cRenderTargetDX();

   void Destroy();

   virtual void SwapBuffers();

private:
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERTARGETDX_H
