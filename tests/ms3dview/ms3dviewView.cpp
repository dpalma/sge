/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"
#include "ms3dview.h"

#include "ms3dviewDoc.h"
#include "ms3dviewView.h"

#include "entityapi.h"
#include "model.h"
#include "renderapi.h"

#include "globalobj.h"
#include "matrix4.h"

#include <cfloat>
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const GLfloat kFov = 90;
const GLfloat kZNear = 1;
const GLfloat kZFar = 2000;

/////////////////////////////////////////////////////////////////////////////
// c3dmodelView

IMPLEMENT_DYNCREATE(c3dmodelView, CView)

BEGIN_MESSAGE_MAP(c3dmodelView, CView)
	//{{AFX_MSG_MAP(c3dmodelView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// c3dmodelView construction/destruction

c3dmodelView::c3dmodelView() : m_center(0,0,0), m_eye(0,0,0)
{
}

c3dmodelView::~c3dmodelView()
{
}

BOOL c3dmodelView::PreCreateWindow(CREATESTRUCT& cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// c3dmodelView operations

void c3dmodelView::OnFrame(double time, double elapsed)
{
   UseGlobal(Renderer);
   Verify(pRenderer->BeginScene() == S_OK);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_center.x, m_center.y, m_center.z, 0, 1, 0);
   glTranslatef(m_center.x, m_center.y, m_center.z);

   glPopMatrix();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

   pRenderer->EndScene();
   glFinish();
   SwapBuffers(m_hDC);
}


/////////////////////////////////////////////////////////////////////////////
// c3dmodelView drawing

void c3dmodelView::OnDraw(CDC* pDC)
{
	c3dmodelDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}

/////////////////////////////////////////////////////////////////////////////
// c3dmodelView diagnostics

#ifdef _DEBUG
void c3dmodelView::AssertValid() const
{
	CView::AssertValid();
}

void c3dmodelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

c3dmodelDoc* c3dmodelView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(c3dmodelDoc)));
	return (c3dmodelDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// c3dmodelView message handlers

int c3dmodelView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

   cMs3dviewApp * pApp = DYNAMIC_DOWNCAST(cMs3dviewApp, AfxGetApp());
   pApp->AddLoopClient(static_cast<ms3dview::cFrameLoopClient*>(this));
	
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
   if (pixelFormat == 0)
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

   COLORREF colorWindow = GetSysColor(COLOR_WINDOW);

   glClearColor(
      (float)GetRValue(colorWindow) / 255,
      (float)GetGValue(colorWindow) / 255,
      (float)GetBValue(colorWindow) / 255,
      1);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

	return 0;
}

void c3dmodelView::OnDestroy() 
{
	CView::OnDestroy();

   cMs3dviewApp * pApp = DYNAMIC_DOWNCAST(cMs3dviewApp, AfxGetApp());
   pApp->RemoveLoopClient(static_cast<ms3dview::cFrameLoopClient*>(this));

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

void c3dmodelView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

   GLfloat aspect = static_cast<GLfloat>(cx) / cy;

   tMatrix4 proj;
   MatrixPerspective(kFov, aspect, kZNear, kZFar, &proj);

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(proj.m);

   glViewport(0, 0, cx, cy);
}

BOOL c3dmodelView::OnEraseBkgnd(CDC* pDC) 
{
   return TRUE;
}

static bool operator <(const tVec3 & a, const tVec3 & b)
{
   return (a.x < b.x) && (a.y < b.y) && (a.z < b.z);
}

static bool operator >(const tVec3 & a, const tVec3 & b)
{
   return (a.x > b.x) && (a.y > b.y) && (a.z > b.z);
}

void c3dmodelView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	c3dmodelDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   if (pDoc && pDoc->AccessModel())
   {
      tVec3 maximum(FLT_MIN, FLT_MIN, FLT_MIN),
            minimum(FLT_MAX, FLT_MAX, FLT_MAX);

      const tModelVertices & vertices = pDoc->AccessModel()->GetVertices();
      tModelVertices::const_iterator iter = vertices.begin();
      for (; iter != vertices.end(); iter++)
      {
         if (iter->pos < minimum)
            minimum = iter->pos;
         if (iter->pos > maximum)
            maximum = iter->pos;
      }

      float maxDim = Max(maximum.x - minimum.x, Max(maximum.y - minimum.y, maximum.z - minimum.z));

      m_center = (maximum + minimum) * 0.5f;
      tVec3 eyeOffset(maxDim, maxDim * 2, maxDim * 5);
      m_eye = m_center + eyeOffset;
   }
   else
   {
      m_center = tVec3(0,0,0);
      m_eye = tVec3(0,0,0);
   }
}
