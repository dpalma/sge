///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef _WIN32
#error ("This file is for Windows compilation only")
#endif

#include "stdhdr.h"

#include "rendertargetw32.h"

#include <GL/glew.h>
#include <GL/wglew.h>

#include "tech/dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
// Create a default OpenGL context using the Win32 function ChoosePixelFormat

static int CreateDefaultContext(HWND hWnd, int * pBpp, HDC * phDC, HGLRC * phGLRC)
{
   Assert(IsWindow(hWnd));

   *phDC = GetDC(hWnd);
   if (!*phDC)
   {
      return 0;
   }

   if (*pBpp == 0)
   {
      *pBpp = GetDeviceCaps(*phDC, BITSPIXEL);
   }

   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = *pBpp;
   pfd.cDepthBits = *pBpp;
   pfd.cStencilBits = *pBpp;
   pfd.dwLayerMask = PFD_MAIN_PLANE;

   int pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   while ((pixelFormat == 0) && (pfd.cStencilBits > 0))
   {
      pfd.cStencilBits /= 2;
      pixelFormat = ChoosePixelFormat(*phDC, &pfd);
   }

   if (pixelFormat == 0)
   {
      ErrorMsg("Unable to find a suitable pixel format\n");
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   if (!SetPixelFormat(*phDC, pixelFormat, &pfd))
   {
      ErrorMsg1("SetPixelFormat failed with error %d\n", GetLastError());
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   *phGLRC = wglCreateContext(*phDC);
   if ((*phGLRC) == NULL)
   {
      ErrorMsg1("wglCreateContext failed with error 0x%04x\n", glGetError());
      ReleaseDC(hWnd, *phDC);
      *phDC = NULL;
      *phGLRC = NULL;
      return 0;
   }

   return pixelFormat;
}


////////////////////////////////////////////////////////////////////////////////

tResult RenderTargetW32Create(HWND hWnd, IRenderTarget * * ppRenderTarget)
{
   if (ppRenderTarget == NULL)
   {
      return E_POINTER;
   }

   if (!IsWindow(hWnd))
   {
      return E_INVALIDARG;
   }

   int bpp = 0;
   HDC hDC = NULL;
   HGLRC hGLRC = NULL;

   if (CreateDefaultContext(hWnd, &bpp, &hDC, &hGLRC) == 0)
   {
      ErrorMsg("An error occurred creating the GL context\n");
      return E_FAIL;
   }

   if (!wglMakeCurrent(hDC, hGLRC))
   {
      GLenum glError = glGetError();
      ErrorMsg1("wglMakeCurrent failed with error 0x%04x\n", glError);
      ReleaseDC(hWnd, hDC);
      wglDeleteContext(hGLRC);
      return E_FAIL;
   }

   if (glewInit() != GLEW_OK)
   {
      ErrorMsg("GL extension manager failed to initialize\n");
      ReleaseDC(hWnd, hDC);
      wglDeleteContext(hGLRC);
      return E_FAIL;
   }

   cAutoIPtr<IRenderTarget> pTarget(static_cast<IRenderTarget*>(new cRenderTargetW32(hWnd, hDC, hGLRC)));
   if (!pTarget)
   {
      return E_OUTOFMEMORY;
   }

   return pTarget.GetPointer(ppRenderTarget);
}


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cRenderTargetW32
//

////////////////////////////////////////

cRenderTargetW32::cRenderTargetW32(HWND hWnd, HDC hDC, HGLRC hGLRC)
 : m_hWnd(hWnd)
 , m_hDC(hDC)
 , m_hGLRC(hGLRC)
{
}

////////////////////////////////////////

cRenderTargetW32::~cRenderTargetW32()
{
   Destroy();
}

////////////////////////////////////////

void cRenderTargetW32::Destroy()
{
   if (m_hDC != NULL)
   {
      wglMakeCurrent(m_hDC, NULL);
      ReleaseDC(m_hWnd, m_hDC);
      m_hDC = NULL;
   }

   if (m_hGLRC != NULL)
   {
      wglDeleteContext(m_hGLRC);
      m_hGLRC = NULL;
   }

   m_hWnd = NULL;
}

////////////////////////////////////////

void cRenderTargetW32::SwapBuffers()
{
   Verify(::SwapBuffers(m_hDC));
}

///////////////////////////////////////////////////////////////////////////////
