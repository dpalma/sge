/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_GLCONTEXT_H)
#define INCLUDED_GLCONTEXT_H

#include <atluser.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGLContext
//

template <class T>
class cGLContext
{
public:
   cGLContext() : m_hDC(NULL), m_hRC(NULL)
   {
   }

   ~cGLContext()
   {
   }

   BEGIN_MSG_MAP(cGLContext)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
      T * pT = static_cast<T*>(this);

      LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

      if (!(lpCreateStruct->style & (WS_CLIPCHILDREN | WS_CLIPSIBLINGS)))
      {
         ErrorMsg("WS_CLIPCHILDREN and WS_CLIPSIBLINGS styles are required\n");
         return -1;
      }

      m_hDC = ::GetDC(pT->m_hWnd);
      if (m_hDC == NULL)
      {
         return -1;
      }

      PIXELFORMATDESCRIPTOR pfd = {0};
      pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      pfd.nVersion = 1;
      pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
      pfd.dwLayerMask = PFD_MAIN_PLANE;
      pfd.iPixelType = PFD_TYPE_RGBA;
      pfd.cColorBits = GetDeviceCaps(m_hDC, BITSPIXEL);

      int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);

      if (!pixelFormat)
      {
         return -1;
      }

      if (pfd.dwFlags & PFD_NEED_PALETTE)
      {
         ErrorMsg("Needs palette\n");
         return -1;
      }

      if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
      {
         return -1;
      }

      m_hRC = wglCreateContext(m_hDC);
      if (m_hRC == NULL)
      {
         return -1;
      }

      wglMakeCurrent(m_hDC, m_hRC);

      bHandled = FALSE;
      return 0;
   }

   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
      T * pT = static_cast<T*>(this);

      wglMakeCurrent(NULL, NULL);

      if (m_hRC != NULL)
      {
         wglDeleteContext(m_hRC);
         m_hRC = NULL;
      }

      if (m_hDC != NULL)
      {
         ::ReleaseDC(pT->m_hWnd, m_hDC);
         m_hDC = NULL;
      }

      bHandled = FALSE;
      return 0;
   }

   LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
      return TRUE;
   }

protected:
   HDC GetSafeHdc() const
   {
      return (this == NULL) ? NULL : m_hDC;
   }

   HGLRC GetSafeHglrc() const
   {
      return (this == NULL) ? NULL : m_hRC;
   }

private:
   HDC m_hDC;
   HGLRC m_hRC;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_GLCONTEXT_H)
