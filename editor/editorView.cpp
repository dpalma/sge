/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"
#include "terrainapi.h"
#include "editorapi.h"
#include "editorTools.h"
#include "editorTypes.h"
#include "CameraDlg.h"

#include "cameraapi.h"
#include "entityapi.h"
#include "ray.h"
#include "renderapi.h"

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
	ON_COMMAND(ID_TOOLS_CAMERASETTINGS, OnToolsCameraSettings)
   ON_NOTIFY_RANGE(TTN_SHOW, 0, 0xFFFFFFFF, OnToolTipShow)
   ON_NOTIFY_RANGE(TTN_POP, 0, 0xFFFFFFFF, OnToolTipPop)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

////////////////////////////////////////

cEditorView::cEditorView()
 : m_bInitialized(false)
 , m_bUsingD3d(false)
 , m_hDC(NULL)
 , m_hRC(NULL)
#ifdef HAVE_DIRECTX
 , m_d3d9Lib("d3d9")
#endif
 , m_cameraFov(kFov)
 , m_cameraZNear(kZNear)
 , m_cameraZFar(kZFar)
 , m_bInPostNcDestroy(false)
{
}

////////////////////////////////////////

cEditorView::~cEditorView()
{
}

////////////////////////////////////////

BOOL cEditorView::PreCreateWindow(CREATESTRUCT & cs)
{
   cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC, LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

   // Style bits required by OpenGL
   cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

////////////////////////////////////////

tResult cEditorView::OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool)
{
   UseGlobal(TerrainRenderer);
   pTerrainRenderer->ClearHighlight();
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::OnDefaultTileSetChange(const tChar * pszTileSet)
{
   return S_OK;
}

////////////////////////////////////////

void cEditorView::OnFrame(double time, double elapsed)
{
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
   UseGlobal(Renderer);
   Verify(pRenderer->BeginScene() == S_OK);

   UseGlobal(Camera);

   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(pCamera->GetViewMatrix().m);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

   pRenderer->EndScene();
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

      UseGlobal(Camera);
      pCamera->SetPerspective(m_cameraFov, aspect, m_cameraZNear, m_cameraZFar);

      if (m_bUsingD3d)
      {
#ifdef HAVE_DIRECTX
         m_presentParams.BackBufferWidth = cx;
         m_presentParams.BackBufferHeight = cy;
         m_pD3dDevice->Reset(&m_presentParams);

         D3DXMATRIX projTest;
         D3DXMatrixPerspectiveFovRH(&projTest, m_cameraFov, aspect, m_cameraZNear, m_cameraZFar);

         D3DMATRIX proj;
         MatrixTranspose(m_proj, &proj);
         m_pD3dDevice->SetTransform(D3DTS_PROJECTION, &proj);
#endif
      }
      else
      {
         glViewport(0, 0, cx, cy);

         glMatrixMode(GL_PROJECTION);
         glLoadMatrixf(pCamera->GetProjectionMatrix().m);
      }
   }
}

////////////////////////////////////////

BOOL cEditorView::OnEraseBkgnd(CDC * pDC) 
{
   return TRUE;
}

////////////////////////////////////////

void cEditorView::OnToolsCameraSettings()
{
   cCameraDlg cameraDlg(this);

   UseGlobal(CameraControl);

   cameraDlg.m_fov = Round(m_cameraFov);
   cameraDlg.m_znear = Round(m_cameraZNear);
   cameraDlg.m_zfar = Round(m_cameraZFar);
   float temp;
   pCameraControl->GetElevation(&temp);
   cameraDlg.m_elevation = Round(temp);
   pCameraControl->GetPitch(&temp);
   cameraDlg.m_pitch = Round(temp);

   if (cameraDlg.DoModal() == IDOK)
   {
      m_cameraFov = static_cast<float>(cameraDlg.m_fov);
      m_cameraZNear = static_cast<float>(cameraDlg.m_znear);
      m_cameraZFar = static_cast<float>(cameraDlg.m_zfar);
      pCameraControl->SetElevation(static_cast<float>(cameraDlg.m_elevation));
      pCameraControl->SetPitch(static_cast<float>(cameraDlg.m_pitch));

      CRect clientRect;
      GetClientRect(clientRect);

      float aspect = static_cast<float>(clientRect.Width()) / clientRect.Height();

      UseGlobal(Camera);
      pCamera->SetPerspective(m_cameraFov, aspect, m_cameraZNear, m_cameraZFar);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(pCamera->GetProjectionMatrix().m);
   }
}

////////////////////////////////////////

void cEditorView::OnToolTipShow(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
   // Without this code, tooltips will appear briefly over the window, but
   // get clobbered by the OpenGL rendering loop. Since the actual tooltip
   // window that shows up on-screen is not a child nor sibling of this
   // window, the WS_CLIPCHILDREN and WS_CLIPSIBLINGS do not prevent this.
   // The solution here is to create a window the exact size of the tooltip
   // that IS a child of this window and position it so that it defends the
   // tooltip against being drawn over by OpenGL.
   RECT rect;
   ::GetWindowRect(pNMHDR->hwndFrom, &rect);
   ::MapWindowPoints(NULL, m_hWnd, reinterpret_cast<LPPOINT>(&rect), 2);
   if (IsWindow(m_toolTipGuard.m_hWnd))
   {
      m_toolTipGuard.MoveWindow(&rect);
      m_toolTipGuard.ShowWindow(SW_SHOW);
   }
   else
   {
      Verify(m_toolTipGuard.Create(NULL, NULL, WS_VISIBLE, rect, this, 0));
   }
   ::SetWindowPos(pNMHDR->hwndFrom, m_toolTipGuard.m_hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

////////////////////////////////////////

void cEditorView::OnToolTipPop(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
   m_toolTipGuard.ShowWindow(SW_HIDE);
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

   UseGlobal(CameraControl);
   pCameraControl->LookAtPoint(centerX, centerZ);

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

void cEditorView::PreSubclassWindow()
{
   CScrollView::PreSubclassWindow();

   EnableToolTips();
}

////////////////////////////////////////

int cEditorView::OnToolHitTest(CPoint point, TOOLINFO * pToolInfo) const
{
   UseGlobal(EditorApp);
   cAutoIPtr<IEditorTool> pActiveTool;
   if (pEditorApp->GetActiveTool(&pActiveTool) == S_OK)
   {
      cStr toolTipText;
      uint_ptr toolTipId = 0;
      if (pActiveTool->GetToolTip(cEditorMouseEvent(point), &toolTipText, &toolTipId) == S_OK)
      {
         pToolInfo->hwnd = m_hWnd;
         pToolInfo->uFlags |= TTF_NOTBUTTON;
         pToolInfo->uId = toolTipId;
         pToolInfo->lpszText = _tcsdup(toolTipText.c_str()); // MFC frees this memory
         SetRect(&pToolInfo->rect, point.x - 1, point.y - 1, point.x + 1, point.y + 1);
         return point.x * point.y;
      }
   }

   return CScrollView::OnToolHitTest(point, pToolInfo);
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

///////////////////////////////////////////////////////////////////////////////
