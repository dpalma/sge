/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"
#include "terrain.h"
#include "editorapi.h"
#include "editorTools.h"

#include "sceneapi.h"
#include "ray.h"

#include "renderapi.h"

#include "globalobj.h"
#include "keys.h"
#include "techtime.h"

#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
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

IMPLEMENT_DYNCREATE_EX(cEditorView, CView)

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cEditorView, cGLView)
	//{{AFX_MSG_MAP(cEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

////////////////////////////////////////

cEditorView::cEditorView()
 : m_cameraElevation(kDefaultCameraElevation),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_bRecalcEye(true),
   m_highlitTileX(-1),
   m_highlitTileZ(-1)
{
}

////////////////////////////////////////

cEditorView::~cEditorView()
{
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

////////////////////////////////////////

tResult cEditorView::Create(const sWindowCreateParams * /*pParams*/)
{
   Assert(!"This should never be called");
   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorView::SwapBuffers()
{
   return ::SwapBuffers(GetSafeHdc()) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorView::GetCamera(ISceneCamera * * ppCamera)
{
   return m_pCamera.GetPointer(ppCamera);
}

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

tResult cEditorView::SetModel(IEditorModel * pModel)
{
   //SafeRelease(m_pModel);
   //m_pModel = CTAddRef(pModel);
   //InitialUpdate();
   //ClearTileHighlight();
   //return S_OK;
   return E_NOTIMPL;
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
   if (!!m_pCamera)
   {
      tMatrix4 view;
      MatrixLookAt(GetCameraEyePosition(), m_center, tVec3(0,1,0), &view);
      m_pCamera->SetViewMatrix(view);
   }

   RenderScene();
}

////////////////////////////////////////

void cEditorView::RenderScene()
{
   cAutoIPtr<IRenderDevice> pDevice(CTAddRef(AccessRenderDevice()));
   if (!!pDevice)
   {
      pDevice->BeginScene();
      pDevice->Clear();

      UseGlobal(Scene);
      pScene->Render(pDevice);

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

               glPushAttrib(GL_ENABLE_BIT);
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

      pDevice->EndScene();
   }
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView drawing

void cEditorView::OnDraw(CDC * pDC)
{
//	cEditorDoc * pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

   RenderScene();
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView diagnostics

#ifdef _DEBUG
////////////////////////////////////////

void cEditorView::AssertValid() const
{
	cGLView::AssertValid();
}

////////////////////////////////////////

void cEditorView::Dump(CDumpContext& dc) const
{
	cGLView::Dump(dc);
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
	if (cGLView::OnCreate(lpCreateStruct) == -1)
		return -1;

   if (RenderDeviceCreate(static_cast<IWindow *>(this), &m_pRenderDevice) != S_OK)
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

   AccessEditorApp()->SetDefaultTool(cAutoIPtr<IEditorTool>(new cMoveCameraTool));

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	cGLView::OnDestroy();

   Assert(AccessEditorApp() != NULL);
   AccessEditorApp()->RemoveLoopClient(this);

   SafeRelease(m_pCamera);
   SafeRelease(m_pRenderDevice);
}

////////////////////////////////////////

void cEditorView::OnSize(UINT nType, int cx, int cy) 
{
	cGLView::OnSize(nType, cx, cy);

   float aspect = (float)cx / cy;

   if (AccessRenderDevice() != NULL)
   {
      AccessRenderDevice()->SetViewportSize(cx, cy);
   }

   if (!!m_pCamera)
   {
      m_pCamera->SetPerspective(kFov, aspect, kZNear, kZFar);
   }
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

void cEditorView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

}

///////////////////////////////////////////////////////////////////////////////
// The characters mapped here should be handled in WM_KEYDOWN, else in WM_CHAR

static const long g_keyMap[128] =
{
   /*   0 */ 0, kEscape, 0, 0, 0, 0, 0, 0,
   /*   8 */ 0, 0, 0, 0, 0, 0, kBackspace, kTab,
   /*  16 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /*  24 */ 0, 0, 0, 0, kEnter, kCtrl, 0, 0,
   /*  32 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /*  40 */ 0, 0, kLShift, 0, 0, 0, 0, 0,
   /*  48 */ 0, 0, 0, 0, 0, 0, kRShift, 0,
   /*  56 */ kAlt, kSpace, 0, kF1, kF2, kF3, kF4, kF5,
   /*  64 */ kF6, kF7, kF8, kF9, kF10, kPause, 0, kHome,
   /*  72 */ kUp, kPageUp, 0, kLeft, 0, kRight, 0, kEnd,
   /*  80 */ kDown, kPageDown, kInsert, kDelete, 0, 0, 0, kF11,
   /*  88 */ kF12, 0, 0, 0, 0, 0, 0, 0,
   /*  96 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 104 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 112 */ 0, 0, 0, 0, 0, 0, 0, 0,
   /* 120 */ 0, 0, 0, 0, 0, 0, 0, 0,
};

///////////////////////////////////////////////////////////////////////////////
// The keydata parameter is the LPARAM from a Windows key message

static long MapKey(long keydata)
{
   //int repeatCount = keydata & 0xFFFF;

   int scanCode = (keydata >> 16) & 0xFF;

   //bool isExtended = false;
   //if (keydata & (1 << 24))
   //   isExtended = true;

   return g_keyMap[scanCode];
}

////////////////////////////////////////

LRESULT cEditorView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
   double msgTime = TimeGetSecs();

   switch (message)
   {
      case WM_DESTROY:
      {
         ForEachConnection(&IWindowSink::OnDestroy, msgTime);
         break;
      }

      case WM_SIZE:
      {
         ForEachConnection(&IWindowSink::OnResize, (int)LOWORD(lParam), (int)HIWORD(lParam), msgTime);
         break;
      }

      case WM_ACTIVATEAPP:
      {
         ForEachConnection(&IWindowSink::OnActivateApp, wParam ? true : false, msgTime);
         break;
      }

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
      {
         long mapped = MapKey(lParam);
         if (mapped != 0)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, mapped, true, msgTime);
         }
         break;
      }

      case WM_SYSCHAR:
      case WM_CHAR:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            ForEachConnection(&IWindowSink::OnKeyEvent, (long)wParam, true, msgTime);
         }
         break;
      }

      case WM_SYSKEYUP:
      case WM_KEYUP:
      {
         long mapped = MapKey(lParam);
         if (mapped == 0)
         {
            mapped = wParam;
         }
         ForEachConnection(&IWindowSink::OnKeyEvent, mapped, false, msgTime);
         break;
      }

      case WM_MOUSEWHEEL:
      {
         short zDelta = (short)HIWORD(wParam);
         long key = (zDelta < 0) ? kMouseWheelDown : kMouseWheelUp;
         ForEachConnection(&IWindowSink::OnKeyEvent, key, true, msgTime);
         ForEachConnection(&IWindowSink::OnKeyEvent, key, false, msgTime);
         break;
      }

      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MOUSEMOVE:
      {
         uint mouseState = 0;
         if (wParam & MK_LBUTTON)
            mouseState |= kLMouseDown;
         if (wParam & MK_RBUTTON)
            mouseState |= kRMouseDown;
         if (wParam & MK_MBUTTON)
            mouseState |= kMMouseDown;
         ForEachConnection(&IWindowSink::OnMouseEvent, (int)LOWORD(lParam), (int)HIWORD(lParam), mouseState, msgTime);
         break;
      }
   }

	return cGLView::WindowProc(message, wParam, lParam);
}
