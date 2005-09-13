/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"
#include "terrainapi.h"
#include "editorapi.h"
#include "editorTools.h"

#include "ray.h"

#include "configapi.h"
#include "globalobj.h"
#include "techtime.h"
#include "vec4.h"

#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef HAVE_DIRECTX
#pragma comment(lib, "d3dx9")
#endif

/////////////////////////////////////////////////////////////////////////////

typedef IDirect3D9 * (WINAPI * tDirect3DCreate9Fn)(UINT);

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

#ifdef HAVE_DIRECTX
static void MatrixTranspose(const tMatrix4 & m, D3DMATRIX * pT)
{
   pT->_11 = m.m00;
   pT->_12 = m.m10;
   pT->_13 = m.m20;
   pT->_14 = m.m30;
   pT->_21 = m.m01;
   pT->_22 = m.m11;
   pT->_23 = m.m21;
   pT->_24 = m.m31;
   pT->_31 = m.m02;
   pT->_32 = m.m12;
   pT->_33 = m.m22;
   pT->_34 = m.m32;
   pT->_41 = m.m03;
   pT->_42 = m.m13;
   pT->_43 = m.m23;
   pT->_44 = m.m33;
}
#endif


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
 : m_bInitialized(false),
   m_bUsingD3d(false),
   m_hDC(NULL),
   m_hRC(NULL),
#ifdef HAVE_DIRECTX
   m_d3d9Lib("d3d9"),
#endif
   m_cameraElevation(kDefaultCameraElevation),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_bRecalcEye(true),
   m_bUpdateCompositeMatrices(true),
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
   m_bUpdateCompositeMatrices = true;
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
   m_bUpdateCompositeMatrices = true;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::GeneratePickRay(float ndx, float ndy, cRay * pRay)
{
   if (pRay == NULL)
   {
      return E_POINTER;
   }

   UpdateCompositeMatrices();

   tVec4 n;
   m_viewProjInverse.Transform(tVec4(ndx, ndy, -1, 1), &n);
   if (n.w == 0.0f)
   {
      return E_FAIL;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f;
   m_viewProjInverse.Transform(tVec4(ndx, ndy, 1, 1), &f);
   if (f.w == 0.0f)
   {
      return E_FAIL;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   tVec4 eye;
   m_viewInverse.Transform(tVec4(0,0,0,1), &eye);

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

tResult cEditorView::OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool)
{
   ClearTileHighlight();
   return S_OK;
}

////////////////////////////////////////

void cEditorView::OnFrame(double time, double elapsed)
{
   MatrixLookAt(GetCameraEyePosition(), m_center, tVec3(0,1,0), &m_view);
#ifdef HAVE_DIRECTX
   if (m_bUsingD3d)
   {
      RenderD3D();
   }
   else
   {
      RenderGL();
   }
#else
   RenderGL();
#endif
}

////////////////////////////////////////

void cEditorView::RenderGL()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(m_view.m);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   int iHlx, iHlz;
   if (GetHighlightTile(&iHlx, &iHlz) == S_OK)
   {
      UseGlobal(TerrainModel);
      tVec3 corners[4];
      if (pTerrainModel->GetQuadCorners(iHlx, iHlz, corners) == S_OK)
      {
         static const float kOffsetY = 0.5f;
         corners[0].y += kOffsetY;
         corners[1].y += kOffsetY;
         corners[2].y += kOffsetY;
         corners[3].y += kOffsetY;

         glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBegin(GL_QUADS);
            glColor4fv(kHighlightTileColor);
            glNormal3f(0, 1, 0);
            glVertex3fv(corners[0].v);
            glVertex3fv(corners[3].v);
            glVertex3fv(corners[2].v);
            glVertex3fv(corners[1].v);
         glEnd();
         glPopAttrib();
      }
   }

   SwapBuffers(m_hDC);
}

////////////////////////////////////////

void cEditorView::RenderD3D()
{
#ifdef HAVE_DIRECTX
   m_pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);

   if (m_pD3dDevice->BeginScene() == D3D_OK)
   {
      // TODO

      m_pD3dDevice->EndScene();
      m_pD3dDevice->Present(NULL, NULL, NULL, NULL);
   }
#endif
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView drawing

void cEditorView::OnDraw(CDC * pDC)
{
//	cEditorDoc * pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

#ifdef HAVE_DIRECTX
   if (m_bUsingD3d)
   {
      RenderD3D();
   }
   else
   {
      RenderGL();
   }
#else
   RenderGL();
#endif
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


////////////////////////////////////////

bool cEditorView::Initialize()
{
   if (!m_bInitialized)
   {
#ifdef HAVE_DIRECTX
      if (ConfigIsTrue("use_d3d"))
      {
         if (!InitD3D())
         {
            return false;
         }

         m_bUsingD3d = true;
      }
      else
      {
         if (!InitGL())
         {
            return false;
         }
      }
#else
      if (!InitGL())
      {
         return false;
      }
#endif
   }

   m_bInitialized = true;
   return true;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

////////////////////////////////////////

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

   UseGlobal(EditorApp);
   Verify(pEditorApp->AddLoopClient(this) == S_OK);

   // GL can (and should) be initialized during WM_CREATE handling, so do
   // so and set the flag so that later Initialize() calls don't do anything.
   if (!ConfigIsTrue("use_d3d"))
   {
      if (!InitGL())
      {
         return -1;
      }
      m_bInitialized = true;
   }

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	CScrollView::OnDestroy();

   UseGlobal(EditorApp);
   pEditorApp->RemoveLoopClient(this);

#ifdef HAVE_DIRECTX
   SafeRelease(m_pD3dDevice);
   SafeRelease(m_pD3d);
#endif

   m_bUsingD3d = false;

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

   if (!m_bInitialized)
   {
      return;
   }

   // cy cannot be zero because it will be a divisor (in aspect ratio)
   if (cy > 0)
   {
      float aspect = static_cast<float>(cx) / cy;
      MatrixPerspective(kFov, aspect, kZNear, kZFar, &m_proj);

      m_bUpdateCompositeMatrices = true;

      if (m_bUsingD3d)
      {
#ifdef HAVE_DIRECTX
         m_presentParams.BackBufferWidth = cx;
         m_presentParams.BackBufferHeight = cy;
         m_pD3dDevice->Reset(&m_presentParams);

         D3DXMATRIX projTest;
         D3DXMatrixPerspectiveFovRH(&projTest, kFov, aspect, kZNear, kZFar);

         D3DMATRIX proj;
         MatrixTranspose(m_proj, &proj);
         m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &proj);
#endif
      }
      else
      {
         glViewport(0, 0, cx, cy);

         glMatrixMode(GL_PROJECTION);
         glLoadMatrixf(m_proj.m);
      }
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
   // Have to be initialized before placing the camera, for instance, and
   // initialization requires no parameters, so just call it.
   Initialize();

   CRect rect;
   GetClientRect(rect);
   SetScaleToFitSize(rect.Size());

	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   UseGlobal(TerrainModel);
   cTerrainSettings terrainSettings;
   pTerrainModel->GetTerrainSettings(&terrainSettings);

   float centerX = static_cast<float>(terrainSettings.GetTileCountX() * terrainSettings.GetTileSize()) / 2;
   float centerZ = static_cast<float>(terrainSettings.GetTileCountZ() * terrainSettings.GetTileSize()) / 2;

   PlaceCamera(centerX, centerZ);

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

////////////////////////////////////////

bool cEditorView::InitGL()
{
   m_hDC = ::GetDC(m_hWnd);
   if (m_hDC == NULL)
   {
      return false;
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
      return false;
   }

   if (pfd.dwFlags & PFD_NEED_PALETTE)
   {
      ErrorMsg("GL context needs palette\n");
      return false;
   }

   if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
   {
      return false;
   }

   m_hRC = wglCreateContext(m_hDC);
   if (m_hRC == NULL)
   {
      return false;
   }

   if (!wglMakeCurrent(m_hDC, m_hRC))
   {
      return false;
   }

   if (glewInit() != GLEW_OK)
   {
      return false;
   }

   return true;
}

////////////////////////////////////////

bool cEditorView::InitD3D()
{
#ifdef HAVE_DIRECTX
   tDirect3DCreate9Fn pfnDirect3DCreate9 = reinterpret_cast<tDirect3DCreate9Fn>(
      m_d3d9Lib.GetProcAddress("Direct3DCreate9"));
   if (pfnDirect3DCreate9 == NULL)
   {
      return false;
   }

   Assert(!m_pD3d); // This method should be called only once
   m_pD3d = (*pfnDirect3DCreate9)(DIRECT3D_VERSION);
   if (!m_pD3d)
   {
      return false;
   }

   D3DDISPLAYMODE displayMode;
   if (FAILED(m_pD3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
   {
      return false;
   }

   memset(&m_presentParams, 0, sizeof(m_presentParams));
   m_presentParams.BackBufferCount = 1;
   m_presentParams.BackBufferFormat = displayMode.Format;
   m_presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
   m_presentParams.Windowed = TRUE;
   m_presentParams.EnableAutoDepthStencil = TRUE;
   m_presentParams.AutoDepthStencilFormat = D3DFMT_D16;
   m_presentParams.hDeviceWindow = m_hWnd;
   m_presentParams.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | D3DPRESENTFLAG_DEVICECLIP;

   HRESULT hr = m_pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
      D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_presentParams, &m_pD3dDevice);
   if (FAILED(hr))
   {
      hr = m_pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, m_hWnd,
         D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_presentParams, &m_pD3dDevice);
      {
         ErrorMsg1("D3D error %x\n", hr);
         return false;
      }
   }

   return true;
#else
   return false;
#endif
}

////////////////////////////////////////

void cEditorView::UpdateCompositeMatrices()
{
   if (m_bUpdateCompositeMatrices)
   {
      MatrixInvert(m_view.m, m_viewInverse.m);
      m_proj.Multiply(m_view, &m_viewProj);
      MatrixInvert(m_viewProj.m, m_viewProjInverse.m);
      m_bUpdateCompositeMatrices = false;
   }
}

///////////////////////////////////////////////////////////////////////////////
