/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "glView.h"

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cGLView

BEGIN_MESSAGE_MAP(cGLView, CView)
	//{{AFX_MSG_MAP(cGLView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cGLView construction/destruction

cGLView::cGLView()
{
}

cGLView::~cGLView()
{
}

BOOL cGLView::PreCreateWindow(CREATESTRUCT & cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cGLView operations


/////////////////////////////////////////////////////////////////////////////
// cGLView drawing


/////////////////////////////////////////////////////////////////////////////
// cGLView diagnostics

#ifdef _DEBUG
void cGLView::AssertValid() const
{
	CView::AssertValid();
}

void cGLView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cGLView message handlers

int cGLView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

   m_hDC = ::GetDC(m_hWnd);
   if (m_hDC == NULL)
      return -1;

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
      AfxMessageBox("Needs palette");
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

	return 0;
}

void cGLView::OnDestroy() 
{
	CView::OnDestroy();

   wglMakeCurrent(NULL, NULL);

   if (m_hRC != NULL)
   {
      wglDeleteContext(m_hRC);
      m_hRC = NULL;
   }

   if (m_hDC != NULL)
   {
      ::ReleaseDC(m_hWnd, m_hDC);
      m_hDC = NULL;
   }
}

BOOL cGLView::OnEraseBkgnd(CDC * pDC) 
{
   return TRUE;
}
