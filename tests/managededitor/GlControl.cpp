///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "GlControl.h"

#include <GL/glew.h>
#include <windows.h>


namespace Editor
{


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGlControl
//

cGlControl::cGlControl()
 : m_hDc(NULL)
 , m_hGlrc(NULL)
{
}

cGlControl::~cGlControl()
{
}

void cGlControl::SwapBuffers()
{
   if (m_hDc != NULL)
   {
      ::SwapBuffers(m_hDc);
   }
}

void cGlControl::OnHandleCreated(System::EventArgs ^ e)
{
   System::Windows::Forms::UserControl::OnHandleCreated(e);
   DestroyContext();
   CreateContext();
}

void cGlControl::OnHandleDestroyed(System::EventArgs ^ e)
{
   System::Windows::Forms::UserControl::OnHandleDestroyed(e);
   DestroyContext();
}

void cGlControl::OnResize(System::EventArgs ^ e)
{
   System::Windows::Forms::UserControl::OnResize(e);
}

void cGlControl::CreateContext()
{
   Assert(m_hDc == NULL && m_hGlrc == NULL);

   HWND hWnd = reinterpret_cast<HWND>(Handle.ToPointer());

   m_hDc = GetDC(hWnd);

   int bpp = GetDeviceCaps(m_hDc, BITSPIXEL);

   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = bpp;
   pfd.cDepthBits = bpp;
   pfd.cStencilBits = bpp;
   pfd.dwLayerMask = PFD_MAIN_PLANE;

   int pixelFormat = ChoosePixelFormat(m_hDc, &pfd);
   while ((pixelFormat == 0) && (pfd.cStencilBits > 0))
   {
      pfd.cStencilBits /= 2;
      pixelFormat = ChoosePixelFormat(m_hDc, &pfd);
   }

   SetPixelFormat(m_hDc, pixelFormat, &pfd);
   m_hGlrc = wglCreateContext(m_hDc);
   wglMakeCurrent(m_hDc, m_hGlrc);

   glewInit();
}

void cGlControl::DestroyContext()
{
   if (IsHandleCreated)
   {
      HWND hWnd = reinterpret_cast<HWND>(Handle.ToPointer());

      if (m_hDc != NULL)
      {
         wglMakeCurrent(m_hDc, NULL);
         ReleaseDC(hWnd, m_hDc);
         m_hDc = NULL;
      }
   }
   else
   {
      AssertMsg(m_hDc == NULL, "No window handle but HDC is not NULL");
   }

   if (m_hGlrc != NULL)
   {
      wglDeleteContext(m_hGlrc);
      m_hGlrc = NULL;
   }
}


} // namespace Editor

///////////////////////////////////////////////////////////////////////////////
