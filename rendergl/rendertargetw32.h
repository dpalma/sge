///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERTARGETW32_H
#define INCLUDED_RENDERTARGETW32_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_HANDLE(HWND);
F_DECLARE_HANDLE(HDC);
F_DECLARE_HANDLE(HGLRC);

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetW32
//

class cRenderTargetW32 : public cComObject<IMPLEMENTS(IRenderTarget)>
{
   cRenderTargetW32(const cRenderTargetW32 &);
   const cRenderTargetW32 & operator =(const cRenderTargetW32 &);

   friend tResult RenderTargetW32Create(HWND hWnd, IRenderTarget * * ppRenderTarget);

public:
   cRenderTargetW32(HWND hWnd, HDC hDC, HGLRC hGLRC);
   ~cRenderTargetW32();

   void Destroy();

   virtual void SwapBuffers();

private:
   HWND m_hWnd;
   HDC m_hDC;
   HGLRC m_hGLRC;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERTARGETW32_H
