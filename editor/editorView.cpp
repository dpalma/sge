/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorApp.h"
#include "editorDoc.h"
#include "editorView.h"

#include "sceneapi.h"

#include "render.h"

#include "globalobj.h"
#include "matrix4.h"

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const GLfloat kFov = 70;
const GLfloat kZNear = 1;
const GLfloat kZFar = 2000;

/////////////////////////////////////////////////////////////////////////////
// cEditorView

IMPLEMENT_DYNCREATE(cEditorView, CView)

BEGIN_MESSAGE_MAP(cEditorView, CView)
	//{{AFX_MSG_MAP(cEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

cEditorView::cEditorView()
 : m_center(0,0,0),
   m_eye(0,0,0)
{
}

cEditorView::~cEditorView()
{
}

BOOL cEditorView::PreCreateWindow(CREATESTRUCT & cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

void cEditorView::OnFrame(double time, double elapsed)
{
   DebugMsg2("cEditorView::OnFrame(%f, %f)\n", time, elapsed);

   if (!!m_pCamera)
   {
      tMatrix4 view;
      MatrixLookAt(m_eye, m_center, tVec3(0,1,0), &view);
      m_pCamera->SetViewMatrix(view);
   }
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView drawing

void cEditorView::OnDraw(CDC * pDC)
{
	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//   glMatrixMode(GL_MODELVIEW);
//
//   glPushMatrix();
//
//   glLoadIdentity();
//
//   gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_center.x, m_center.y, m_center.z, 0, 1, 0);
//
//   glTranslatef(m_center.x, m_center.y, m_center.z);

   AccessRenderDevice()->BeginScene();
   AccessRenderDevice()->Clear();

   UseGlobal(Scene);
   pScene->Render(AccessRenderDevice());

   AccessRenderDevice()->EndScene();

//   glPopMatrix();
//
//   glFinish();

   SwapBuffers(m_hDC);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView diagnostics

#ifdef _DEBUG
void cEditorView::AssertValid() const
{
	CView::AssertValid();
}

void cEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

cEditorDoc* cEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cEditorDoc)));
	return (cEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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

   if (RenderDeviceCreate(0, &m_pRenderDevice) != S_OK)
   {
      DebugMsg("Failed to create rendering device\n");
      return -1;
   }

   m_pCamera = SceneCameraCreate();
   if (!m_pCamera)
   {
      DebugMsg("ERROR: Failed to create camera\n");
      return -1;
   }

   UseGlobal(Scene);
   pScene->SetCamera(kSL_Terrain, m_pCamera);

   Assert(AccessEditorApp() != NULL);
   Verify(AccessEditorApp()->AddLoopClient(this) == S_OK);

	return 0;
}

void cEditorView::OnDestroy() 
{
	CView::OnDestroy();

   Assert(AccessEditorApp() != NULL);
   Verify(AccessEditorApp()->RemoveLoopClient(this) == S_OK);

   SafeRelease(m_pCamera);

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

void cEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

   GLfloat aspect = (GLfloat)cx / (GLfloat)cy;

   sMatrix4 proj;
   MatrixPerspective(kFov, aspect, kZNear, kZFar, &proj);

   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(proj.m);

   glViewport(0, 0, cx, cy);

   if (AccessRenderDevice() != NULL)
   {
      AccessRenderDevice()->SetViewportSize(cx, cy);
   }

   if (!!m_pCamera)
   {
      m_pCamera->SetPerspective(kFov, (float)cx / cy, kZNear, kZFar);
   }
}

BOOL cEditorView::OnEraseBkgnd(CDC * pDC) 
{
   return TRUE;
}

void cEditorView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   // TODO

   m_center = tVec3(0,0,0);
   m_eye = tVec3(0,0,0);
}
