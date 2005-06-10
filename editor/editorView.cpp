/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"
#include "terrainapi.h"
#include "editorapi.h"
#include "editorTools.h"

#include "ray.h"

#include "globalobj.h"
#include "techtime.h"
#include "vec4.h"

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const float kFov = 70;
const float kZNear = 1;
const float kZFar = 5000;

const float kDefaultCameraElevation = 100;
const float kDefaultCameraPitch = 70;

static const GLfloat kHighlightTileColor[] = { 0, 1, 0, 0.25f };

/////////////////////////////////////////////////////////////////////////////

static tVec3 CalcEyePoint(const tVec3 & center,
                          tVec3::value_type elevation = kDefaultCameraElevation,
                          tVec3::value_type pitch = kDefaultCameraPitch)
{
   return center + tVec3(0, elevation, elevation / tanf(pitch));
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

////////////////////////////////////////

IMPLEMENT_DYNCREATE(cEditorView, CScrollView)

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cEditorView, CScrollView)
	//{{AFX_MSG_MAP(cEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

////////////////////////////////////////

cEditorView::cEditorView()
 : m_hDC(NULL),
   m_hRC(NULL),
   m_cameraElevation(kDefaultCameraElevation),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_bRecalcEye(true),
   m_highlitTileX(-1),
   m_highlitTileZ(-1),
   m_bInPostNcDestroy(false)
{
}

////////////////////////////////////////

cEditorView::~cEditorView()
{
}

////////////////////////////////////////

BOOL cEditorView::PreCreateWindow(CREATESTRUCT & cs)
{
   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

////////////////////////////////////////

tVec3 cEditorView::GetCameraEyePosition() const
{
   if (m_bRecalcEye)
   {
      m_eye = CalcEyePoint(m_center, m_cameraElevation);
      m_bRecalcEye = false;
   }
   return m_eye;
}

////////////////////////////////////////

tResult cEditorView::GetCameraPlacement(float * px, float * pz)
{
   if (px == NULL || pz == NULL)
   {
      return E_POINTER;
   }
   *px = m_center.x;
   *pz = m_center.z;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::PlaceCamera(float x, float z)
{
   m_center.x = x;
   m_center.z = z;
   m_bRecalcEye = true;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::GetCameraElevation(float * pElevation)
{
   if (pElevation == NULL)
   {
      return E_POINTER;
   }

   *pElevation = m_cameraElevation;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::SetCameraElevation(float elevation)
{
   m_cameraElevation = elevation;
   m_bRecalcEye = true;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::GeneratePickRay(float ndx, float ndy, cRay * pRay) const
{
   if (pRay == NULL)
   {
      return E_POINTER;
   }

   tMatrix4 viewProj, viewProjInverse;
   m_proj.Multiply(m_view, &viewProj);
   MatrixInvert(viewProj.m, viewProjInverse.m);

   tVec4 n;
   viewProjInverse.Transform(tVec4(ndx, ndy, -1, 1), &n);
   if (n.w == 0.0f)
   {
      return E_FAIL;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f;
   viewProjInverse.Transform(tVec4(ndx, ndy, 1, 1), &f);
   if (f.w == 0.0f)
   {
      return E_FAIL;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   tMatrix4 viewInverse;
   MatrixInvert(m_view.m, viewInverse.m);

   tVec4 eye;
   viewInverse.Transform(tVec4(0,0,0,1), &eye);

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay(tVec3(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::GetModel(IEditorModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   cEditorDoc * pDoc = GetDocument();

   if (pDoc == NULL)
   {
      *ppModel = NULL;
      return S_FALSE;
   }

   *ppModel = CTAddRef(static_cast<IEditorModel *>(pDoc));
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::GetHighlightTile(int * piTileX, int * piTileZ) const
{
   if (piTileX == NULL || piTileZ == NULL)
   {
      return E_POINTER;
   }
   if (m_highlitTileX < 0 || m_highlitTileZ < 0)
   {
      return S_FALSE;
   }
   *piTileX = m_highlitTileX;
   *piTileZ = m_highlitTileZ;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::HighlightTile(int iTileX, int iTileZ)
{
   m_highlitTileX = iTileX;
   m_highlitTileZ = iTileZ;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::ClearTileHighlight()
{
   m_highlitTileX = -1;
   m_highlitTileZ = -1;
   return S_OK;
}

////////////////////////////////////////

void cEditorView::OnFrame(double time, double elapsed)
{
   MatrixLookAt(GetCameraEyePosition(), m_center, tVec3(0,1,0), &m_view);
   Render();
}

////////////////////////////////////////

void cEditorView::Render()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(m_view.m);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   cAutoIPtr<IEditorModel> pEditModel;
   cAutoIPtr<ITerrainModel> pTerrModel;
   if (GetModel(&pEditModel) == S_OK
      && pEditModel->GetTerrainModel(&pTerrModel) == S_OK)
   {
      int iHlx, iHlz;
      if (GetHighlightTile(&iHlx, &iHlz) == S_OK)
      {
         tVec3 verts[4];
         if (pTerrModel->GetTileVertices(iHlx, iHlz, verts) == S_OK)
         {
            static const float kOffsetY = 0.5f;
            verts[0].y += kOffsetY;
            verts[1].y += kOffsetY;
            verts[2].y += kOffsetY;
            verts[3].y += kOffsetY;

            glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
               glColor4fv(kHighlightTileColor);
               glNormal3f(0, 1, 0);
               glVertex3fv(verts[0].v);
               glVertex3fv(verts[3].v);
               glVertex3fv(verts[2].v);
               glVertex3fv(verts[1].v);
            glEnd();
            glPopAttrib();
         }
      }
   }

   SwapBuffers(m_hDC);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView drawing

void cEditorView::OnDraw(CDC * pDC)
{
//	cEditorDoc * pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

   Render();
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView diagnostics

#ifdef _DEBUG
////////////////////////////////////////

void cEditorView::AssertValid() const
{
	CScrollView::AssertValid();
}

////////////////////////////////////////

void cEditorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

////////////////////////////////////////

cEditorDoc* cEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cEditorDoc)));
	return (cEditorDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

////////////////////////////////////////

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

   m_hDC = ::GetDC(m_hWnd);
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

   UseGlobal(EditorApp);
   Verify(pEditorApp->AddLoopClient(this) == S_OK);

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	CScrollView::OnDestroy();

   UseGlobal(EditorApp);
   pEditorApp->RemoveLoopClient(this);

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

////////////////////////////////////////

void cEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

   // cy cannot be zero because it will be a divisor (in aspect ratio)
   if (cy > 0)
   {
      glViewport(0, 0, cx, cy);

      float aspect = static_cast<float>(cx) / cy;

      MatrixPerspective(kFov, aspect, kZNear, kZFar, &m_proj);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(m_proj.m);
   }
}

////////////////////////////////////////

BOOL cEditorView::OnEraseBkgnd(CDC * pDC) 
{
   return TRUE;
}

////////////////////////////////////////

void cEditorView::OnInitialUpdate() 
{
   CRect rect;
   GetClientRect(rect);
   SetScaleToFitSize(rect.Size());

	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   cAutoIPtr<ITerrainModel> pTerrModel;
   if (pDoc->GetTerrainModel(&pTerrModel) == S_OK)
   {
      uint xExt, zExt;
      pTerrModel->GetExtents(&xExt, &zExt);

      PlaceCamera((float)xExt / 2, (float)zExt / 2);
   }

	CView::OnInitialUpdate();
}

////////////////////////////////////////

void cEditorView::OnFinalRelease()
{
   delete this;
}

////////////////////////////////////////

void cEditorView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}

////////////////////////////////////////

void cEditorView::PostNcDestroy()
{
   // Do not call the base class method which calls "delete this"
   Assert(!m_bInPostNcDestroy);
   m_bInPostNcDestroy = true;
   Release();
}

///////////////////////////////////////////////////////////////////////////////
