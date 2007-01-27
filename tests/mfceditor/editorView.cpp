/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"
#include "engine/terrainapi.h"
#include "editorapi.h"
#include "editorTools.h"
#include "editorTypes.h"
#include "CameraDlg.h"

#include "engine/cameraapi.h"
#include "engine/entityapi.h"
#include "render/renderapi.h"

#include "tech/configapi.h"
#include "tech/globalobj.h"
#include "tech/ray.h"
#include "tech/techtime.h"
#include "tech/vec4.h"

#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const float kFov = 90;
const float kZNear = 1;
const float kZFar = 5000;


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
 : m_cameraFov(kFov)
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

tResult cEditorView::OnDefaultTileSetChange(const tChar * pszTileSet)
{
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool)
{
   UseGlobal(TerrainRenderer);
   pTerrainRenderer->ClearHighlight();
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::Execute(double time)
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
   return S_OK;
}

////////////////////////////////////////

void cEditorView::RenderGL()
{
   UseGlobal(Renderer);
   Verify(pRenderer->BeginScene() == S_OK);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->Render();

   UseGlobal(EntityManager);
   pEntityManager->RenderAll();

   pRenderer->EndScene();
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


/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

////////////////////////////////////////

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

   UseGlobal(Scheduler);
   pScheduler->AddRenderTask(this);

   UseGlobal(EditorToolState);
   Verify(pEditorToolState->AddToolStateListener(static_cast<IEditorToolStateListener*>(this)) == S_OK);

   UseGlobal(Renderer);
   if (pRenderer->CreateContext(m_hWnd) != S_OK)
   {
      return -1;
   }

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	CScrollView::OnDestroy();

   UseGlobal(Scheduler);
   pScheduler->RemoveRenderTask(this);

   UseGlobal(EditorToolState);
   Verify(pEditorToolState->RemoveToolStateListener(static_cast<IEditorToolStateListener*>(this)) == S_OK);

   UseGlobal(Renderer);
   pRenderer->DestroyContext();
}

////////////////////////////////////////

void cEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);

   // cy cannot be zero because it will be a divisor (in aspect ratio)
   if (cy > 0)
   {
      float aspect = static_cast<float>(cx) / cy;

      tMatrix4 proj;
      MatrixPerspective(m_cameraFov, aspect, m_cameraZNear, m_cameraZFar, &proj);

      UseGlobal(Renderer);
      cAutoIPtr<IRenderCamera> pCamera;
      if (pRenderer->GetCamera(&pCamera) == S_OK)
      {
         pCamera->SetProjectionMatrix(proj.m);
      }

      glViewport(0, 0, cx, cy);
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

   cameraDlg.m_fov = FloatToInt(m_cameraFov);
   cameraDlg.m_znear = FloatToInt(m_cameraZNear);
   cameraDlg.m_zfar = FloatToInt(m_cameraZFar);
   float temp;
   pCameraControl->GetElevation(&temp);
   cameraDlg.m_elevation = FloatToInt(temp);
   pCameraControl->GetPitch(&temp);
   cameraDlg.m_pitch = FloatToInt(temp);

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

      tMatrix4 proj;
      MatrixPerspective(m_cameraFov, aspect, m_cameraZNear, m_cameraZFar, &proj);

      UseGlobal(Renderer);
      cAutoIPtr<IRenderCamera> pCamera;
      if (pRenderer->GetCamera(&pCamera) == S_OK)
      {
         pCamera->SetProjectionMatrix(proj.m);
      }
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
   UseGlobal(EditorToolState);
   cAutoIPtr<IEditorTool> pActiveTool;
   if (pEditorToolState->GetActiveTool(&pActiveTool) == S_OK)
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

///////////////////////////////////////////////////////////////////////////////
