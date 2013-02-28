///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERTARGETX11_H
#define INCLUDED_RENDERTARGETX11_H

#include "render/renderapi.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef struct __GLXcontextRec * GLXContext;

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetX11
//

class cRenderTargetX11 : public cComObject<IMPLEMENTS(IRenderTarget)>
{
   cRenderTargetX11(const cRenderTargetX11 &);
   const cRenderTargetX11 & operator =(const cRenderTargetX11 &);

   friend tResult RenderTargetX11Create(Display * display, Window window, IRenderTarget * * ppRenderTarget);

public:
   cRenderTargetX11(Display * display, Window window, GLXContext context);
   ~cRenderTargetX11();

   void Destroy();

   virtual void SwapBuffers();

private:
   Display * m_display;
   Window m_window;
   GLXContext m_context;
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_RENDERTARGETX11_H
