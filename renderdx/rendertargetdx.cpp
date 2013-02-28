///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "rendertargetdx.h"

#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

tResult RenderTargetDXCreate(IDirect3DDevice9 * pD3dDevice, IRenderTarget * * ppRenderTarget)
{
   if (pD3dDevice == NULL)
   {
      return E_POINTER;
   }

   if (ppRenderTarget == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IRenderTarget> pTarget(static_cast<IRenderTarget*>(new cRenderTargetDX(pD3dDevice)));
   if (!pTarget)
   {
      return E_OUTOFMEMORY;
   }

   return pTarget.GetPointer(ppRenderTarget);
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetDX
//

////////////////////////////////////////

cRenderTargetDX::cRenderTargetDX(IDirect3DDevice9 * pD3dDevice)
 : m_pD3dDevice(CTAddRef(pD3dDevice))
{
}

////////////////////////////////////////

cRenderTargetDX::~cRenderTargetDX()
{
   Destroy();
}

////////////////////////////////////////

void cRenderTargetDX::Destroy()
{
   SafeRelease(m_pD3dDevice);
}

////////////////////////////////////////

void cRenderTargetDX::SwapBuffers()
{
}

///////////////////////////////////////////////////////////////////////////////
