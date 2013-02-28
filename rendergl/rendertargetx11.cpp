///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef __GNUC__
#error ("This file is for POSIX/X11 compilation only")
#endif

#include "stdhdr.h"

#include "rendertargetx11.h"

#include <GL/glew.h>
#include <GL/glxew.h>

#include "tech/dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

tResult RenderTargetX11Create(Display * display, Window window, IRenderTarget * * ppRenderTarget)
{
   if (ppRenderTarget == NULL)
   {
      return E_POINTER;
   }

   if (display == NULL)
   {
      return E_POINTER;
   }

   if (window == 0)
   {
      return E_INVALIDARG;
   }

   XWindowAttributes attr = {0};
   XGetWindowAttributes(display, window, &attr);

   if (attr.visual == NULL)
   {
      ErrorMsg("Unable to determine visual for window\n");
      return E_FAIL;
   }

   XVisualInfo vinfoTemplate = {0};
   vinfoTemplate.visual = attr.visual;
   vinfoTemplate.visualid = XVisualIDFromVisual(attr.visual);

   int nVisualInfos = 0;
   XVisualInfo * pVisualInfo = XGetVisualInfo(display, VisualIDMask, &vinfoTemplate, &nVisualInfos);

   if (pVisualInfo == NULL)
   {
      return E_FAIL;
   }

   GLXContext context = glXCreateContext(display, pVisualInfo, None, GL_TRUE);

   XFree(pVisualInfo);

   if (context == NULL)
   {
      ErrorMsg("Could not create glX context\n");
      return E_FAIL;
   }

   glXMakeCurrent(display, window, context);

   if (glewInit() != GLEW_OK)
   {
      ErrorMsg("GL extension manager failed to initialize\n");
      glXDestroyContext(display, context);
      return E_FAIL;
   }

   cAutoIPtr<IRenderTarget> pTarget(static_cast<IRenderTarget*>(new cRenderTargetX11(display, window, context)));
   if (!pTarget)
   {
      return E_OUTOFMEMORY;
   }

   return pTarget.GetPointer(ppRenderTarget);
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetX11
//

////////////////////////////////////////

cRenderTargetX11::cRenderTargetX11(Display * display, Window window, GLXContext context)
 : m_display(display)
 , m_window(window)
 , m_context(context)
{
}

////////////////////////////////////////

cRenderTargetX11::~cRenderTargetX11()
{
   Destroy();
}

////////////////////////////////////////

void cRenderTargetX11::Destroy()
{
   if (m_context != NULL)
   {
      glXDestroyContext(m_display, m_context);
      m_display = NULL;
      m_window = NULL;
      m_context = NULL;
   }
}

////////////////////////////////////////

void cRenderTargetX11::SwapBuffers()
{
   glXSwapBuffers(m_display, m_window);
}

///////////////////////////////////////////////////////////////////////////////
