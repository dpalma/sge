/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ShaderTestApp.h"
#include "ShaderTestDoc.h"
#include "ShaderTestView.h"

#include "engine/entityapi.h"
#include "engine/modelapi.h"
#include "engine/modeltypes.h"
#include "render/renderapi.h"

#include "tech/axisalignedbox.h"
#include "tech/globalobj.h"
#include "tech/matrix4.h"

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
// cShaderTestView

IMPLEMENT_DYNCREATE(cShaderTestView, CView)

BEGIN_MESSAGE_MAP(cShaderTestView, CView)
	//{{AFX_MSG_MAP(cShaderTestView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cShaderTestView construction/destruction

cShaderTestView::cShaderTestView()
 : m_center(0,0,0)
 , m_eye(0,0,0)
{
}

cShaderTestView::~cShaderTestView()
{
}

BOOL cShaderTestView::PreCreateWindow(CREATESTRUCT& cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestView operations

void cShaderTestView::OnFrame(double time, double elapsed)
{
   UseGlobal(Renderer);
   if (pRenderer->BeginScene() == S_OK)
   {
//      glMatrixMode(GL_MODELVIEW);
//      glPushMatrix();
//      glLoadIdentity();
//      gluLookAt(m_eye.x, m_eye.y, m_eye.z, m_center.x, m_center.y, m_center.z, 0, 1, 0);
//      glTranslatef(m_center.x, m_center.y, m_center.z);
//      glPopMatrix();

	   cShaderTestDoc* pDoc = GetDocument();
	   ASSERT_VALID(pDoc);

      cAutoIPtr<IEntityRenderComponent> pRenderComponent;
      if (pDoc->GetModelRenderer(&pRenderComponent) == S_OK)
      {
         pRenderComponent->Update(elapsed);
         pRenderComponent->Render(kERF_None);
      }

      //cAutoIPtr<IModel> pModel;
      //if (pDoc->GetModel(&pModel) == S_OK)
      //{
      //}

      pRenderer->EndScene();

      SwapBuffers(m_hDC);
   }
}


/////////////////////////////////////////////////////////////////////////////
// cShaderTestView drawing

void cShaderTestView::OnDraw(CDC* pDC)
{
	cShaderTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}

/////////////////////////////////////////////////////////////////////////////
// cShaderTestView diagnostics

#ifdef _DEBUG
void cShaderTestView::AssertValid() const
{
	CView::AssertValid();
}

void cShaderTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

cShaderTestDoc* cShaderTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cShaderTestDoc)));
	return (cShaderTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cShaderTestView message handlers

int cShaderTestView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

   cShaderTestApp * pApp = DYNAMIC_DOWNCAST(cShaderTestApp, AfxGetApp());
   pApp->AddLoopClient(static_cast<ShaderTest::cFrameLoopClient*>(this));

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

void cShaderTestView::OnDestroy() 
{
	CView::OnDestroy();

   cShaderTestApp * pApp = DYNAMIC_DOWNCAST(cShaderTestApp, AfxGetApp());
   pApp->RemoveLoopClient(static_cast<ShaderTest::cFrameLoopClient*>(this));

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

void cShaderTestView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

   if (cy > 0)
   {
      GLfloat aspect = static_cast<GLfloat>(cx) / cy;

      tMatrix4 proj;
      MatrixPerspective(kFov, aspect, kZNear, kZFar, &proj);

      UseGlobal(Renderer);
      pRenderer->SetProjectionMatrix(proj.m);
   }

   glViewport(0, 0, cx, cy);
}

BOOL cShaderTestView::OnEraseBkgnd(CDC* pDC) 
{
   return TRUE;
}

static void CalculateBBox(uint nVertices, const sModelVertex * pVertices, tAxisAlignedBox * pBBox)
{
   tVec3 mins(FLT_MAX, FLT_MAX, FLT_MAX), maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   const sModelVertex * pVertex = pVertices;
   for (uint i = 0; i < nVertices; ++i, ++pVertex)
   {
      if (pVertex->pos.x < mins.x)
      {
         mins.x = pVertex->pos.x;
      }
      if (pVertex->pos.y < mins.y)
      {
         mins.y = pVertex->pos.y;
      }
      if (pVertex->pos.z < mins.z)
      {
         mins.z = pVertex->pos.z;
      }
      if (pVertex->pos.x > maxs.x)
      {
         maxs.x = pVertex->pos.x;
      }
      if (pVertex->pos.y > maxs.y)
      {
         maxs.y = pVertex->pos.y;
      }
      if (pVertex->pos.z > maxs.z)
      {
         maxs.z = pVertex->pos.z;
      }
   }
   *pBBox = tAxisAlignedBox(mins, maxs);
}

void cShaderTestView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	cShaderTestDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
   if (pDoc == NULL)
   {
      return;
   }

   m_center = tVec3(0,0,0);
   m_eye = tVec3(0,0,0);

   cAutoIPtr<IModel> pModel;
   if (pDoc->GetModel(&pModel) == S_OK)
   {
      uint nVertices = 0;
      const sModelVertex * pVertices = NULL;
      if (pModel->GetVertices(&nVertices, &pVertices) == S_OK)
      {
         tAxisAlignedBox bbox;
         CalculateBBox(nVertices, pVertices, &bbox);

         bbox.GetCentroid(&m_center);

         float maxDim = bbox.GetBiggestDimension();

         tVec3 eyeOffset(maxDim, maxDim * 2, maxDim * 5);
         m_eye = m_center + eyeOffset;
      }
   }
}
