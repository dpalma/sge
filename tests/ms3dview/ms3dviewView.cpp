// ms3dviewView.cpp : implementation of the CMs3dviewView class
//

#include "stdafx.h"
#include "ms3dview.h"

#include "ms3dviewDoc.h"
#include "ms3dviewView.h"

#include "render.h"
#include "matrix4.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/glext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const GLfloat kFov = 90;
const GLfloat kZNear = 1;
const GLfloat kZFar = 2000;

const int IDC_SLIDER = 1000;
const int kSliderHeight = 30;

PFNGLVERTEXWEIGHTFEXTPROC glVertexWeightfEXT = NULL;
PFNGLVERTEXWEIGHTFVEXTPROC glVertexWeightfvEXT = NULL;
PFNGLVERTEXWEIGHTPOINTEREXTPROC glVertexWeightPointerEXT = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView

IMPLEMENT_DYNCREATE(CMs3dviewView, CView)

BEGIN_MESSAGE_MAP(CMs3dviewView, CView)
	//{{AFX_MSG_MAP(CMs3dviewView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView construction/destruction

CMs3dviewView::CMs3dviewView() : m_center(0,0,0), m_eye(0,0,0)
{
}

CMs3dviewView::~CMs3dviewView()
{
}

BOOL CMs3dviewView::PreCreateWindow(CREATESTRUCT& cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView operations


/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView drawing

void CMs3dviewView::OnDraw(CDC* pDC)
{
	CMs3dviewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);

   glPushMatrix();

   glLoadIdentity();

   gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_center.x, m_center.y, m_center.z, 0, 1, 0);

   glTranslatef(m_center.x, m_center.y, m_center.z);

   if (pDoc->GetModel() != NULL)
   {
      pDoc->GetModel()->Render(NULL);
   }

   glPopMatrix();

   glFinish();

   SwapBuffers(m_hDC);
}

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView diagnostics

#ifdef _DEBUG
void CMs3dviewView::AssertValid() const
{
	CView::AssertValid();
}

void CMs3dviewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMs3dviewDoc* CMs3dviewView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMs3dviewDoc)));
	return (CMs3dviewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMs3dviewView message handlers

int CMs3dviewView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

   if (!m_slider.Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_SLIDER))
   {
      TRACE0("Failed to create slider control\n");
      return -1;
   }

   if (RenderDeviceCreate(&m_pRenderDevice) != S_OK)
   {
      TRACE0("Failed to create rendering device\n");
      return -1;
   }
	
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
      return -1;

   if (pfd.dwFlags & PFD_NEED_PALETTE)
   {
      AfxMessageBox("Needs palette");
      return -1;
   }

   if (!SetPixelFormat(m_hDC, pixelFormat, &pfd)) 
      return -1;

   m_hRC = wglCreateContext(m_hDC);
   if (m_hRC == NULL)
      return -1;

   wglMakeCurrent(m_hDC, m_hRC);

   COLORREF colorWindow = GetSysColor(COLOR_WINDOW);

   glClearColor(
      (float)GetRValue(colorWindow) / 255,
      (float)GetGValue(colorWindow) / 255,
      (float)GetBValue(colorWindow) / 255,
      1);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_VERTEX_WEIGHTING_EXT);
   glEnable(GL_NORMALIZE); // required with vertex weighting

   glVertexWeightfEXT = (PFNGLVERTEXWEIGHTFEXTPROC)wglGetProcAddress("glVertexWeightfEXT");
   glVertexWeightfvEXT = (PFNGLVERTEXWEIGHTFVEXTPROC)wglGetProcAddress("glVertexWeightfvEXT");
   glVertexWeightPointerEXT = (PFNGLVERTEXWEIGHTPOINTEREXTPROC)wglGetProcAddress("glVertexWeightPointerEXT");

	return 0;
}

void CMs3dviewView::OnDestroy() 
{
	CView::OnDestroy();

   SafeRelease(m_pRenderDevice);

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

void CMs3dviewView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

   m_slider.MoveWindow(0, cy - kSliderHeight, cx, kSliderHeight);

   GLfloat aspect = (GLfloat)cx / (GLfloat)cy;

   sMatrix4 proj;
   MatrixPerspective(kFov, aspect, kZNear, kZFar, &proj);

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(proj.m);

   glViewport(0, kSliderHeight, cx, cy - kSliderHeight);
}

BOOL CMs3dviewView::OnEraseBkgnd(CDC* pDC) 
{
   return TRUE;
}

void CMs3dviewView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
   ASSERT_VALID(pScrollBar);
   if (pScrollBar->GetDlgCtrlID() == m_slider.GetDlgCtrlID())
   {
      float percent = (float)(m_slider.GetPos() - m_slider.GetRangeMin()) /
         (m_slider.GetRangeMax() - m_slider.GetRangeMin());

      //TRACE1("Animate to %.2f\n", percent);

	   CMs3dviewDoc * pDoc = GetDocument();
	   ASSERT_VALID(pDoc);

      if (pDoc && pDoc->GetModel())
      {
         pDoc->GetModel()->SetFrame(percent);
         Invalidate(FALSE);
      }
   }
	
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMs3dviewView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	CMs3dviewDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   if (pDoc && pDoc->GetModel())
   {
      tVec3 max, min;
      pDoc->GetModel()->GetAABB(&max, &min);

      float maxDim = Max(max.x - min.x, Max(max.y - min.y, max.z - min.z));

      m_center = (max + min) * 0.5f;
      m_eye = tVec3(m_center.x + maxDim, m_center.y + maxDim, m_center.z + maxDim);
   }
   else
   {
      m_center = tVec3(0,0,0);
      m_eye = tVec3(0,0,0);
   }
}
