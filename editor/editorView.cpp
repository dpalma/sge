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
#include <zmouse.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

const uint WM_GET_IEDITORVIEW = RegisterWindowMessage("WM_GET_IEDITORVIEW");

const float kFov = 70;
const float kZNear = 1;
const float kZFar = 5000;

const float kDefaultCameraElevation = 100;
const float kDefaultCameraPitch = 70;

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

#if _ATL_VER >= 0x0700
OBJECT_ENTRY_AUTO(CLSID_EditorView, cEditorView)
#endif

////////////////////////////////////////

cEditorView::cEditorView()
 : m_cameraElevation(kDefaultCameraElevation),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_bRecalcEye(true),
   m_sceneEntity(this),
   m_highlitTileX(-1),
   m_highlitTileZ(-1)
{
}

////////////////////////////////////////

cEditorView::~cEditorView()
{
}

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

tResult cEditorView::Create(HWND hWndParent, HWND * phWnd)
{
   if (phWnd == NULL)
   {
      return E_POINTER;
   }

   if (!tWindowImplBase::Create(hWndParent, rcDefault))
   {
      return E_FAIL;
   }

   *phWnd = m_hWnd;
   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::Destroy()
{
   return DestroyWindow() ? S_OK : S_FALSE;
}

////////////////////////////////////////

tResult cEditorView::Move(int x, int y, int width, int height)
{
   return MoveWindow(x, y, width, height) ? S_OK : E_FAIL;
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
   return m_pModel.GetPointer(ppModel);
}

////////////////////////////////////////

tResult cEditorView::SetModel(IEditorModel * pModel)
{
   SafeRelease(m_pModel);
   m_pModel = CTAddRef(pModel);
   InitialUpdate();
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

      pDevice->EndScene();
   }
}

////////////////////////////////////////

LRESULT cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (RenderDeviceCreate(static_cast<IWindow *>(this), &m_pRenderDevice) != S_OK)
   {
      ErrorMsg("Failed to create rendering device\n");
      return -1;
   }

   m_pCamera = SceneCameraCreate();
   if (!m_pCamera)
   {
      ErrorMsg("Failed to create camera\n");
      return -1;
   }

   UseGlobal(Scene);
   pScene->SetCamera(kSL_Terrain, m_pCamera);

   pScene->AddEntity(kSL_Terrain, &m_sceneEntity);

   UseGlobal(EditorApp);
   Verify(pEditorApp->AddLoopClient(this) == S_OK);

   pEditorApp->SetDefaultTool(cAutoIPtr<IEditorTool>(new cMoveCameraTool));

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
   UseGlobal(Scene);
   pScene->RemoveEntity(kSL_Terrain, &m_sceneEntity);

   UseGlobal(EditorApp);
   pEditorApp->RemoveLoopClient(this);

   SafeRelease(m_pCamera);
   SafeRelease(m_pRenderDevice);
}

////////////////////////////////////////

void cEditorView::OnSize(UINT nType, CSize size) 
{
   if (size.cy > 0)
   {
      float aspect = (float)size.cx / size.cy;

      if (AccessRenderDevice() != NULL)
      {
         AccessRenderDevice()->SetViewportSize(size.cx, size.cy);
      }

      if (!!m_pCamera)
      {
         m_pCamera->SetPerspective(kFov, aspect, kZNear, kZFar);
      }
   }
}

////////////////////////////////////////

void cEditorView::OnPaint(HDC hDc)
{
   CPaintDC dc(m_hWnd);

   RenderScene();
}

////////////////////////////////////////

LRESULT cEditorView::OnGetIEditorView(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   bHandled = TRUE;
   if (lParam != 0)
   {
      *reinterpret_cast<void**>(lParam) = static_cast<IEditorView *>(this);
      AddRef();
      return 0;
   }
   return -1;
}

/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////

cEditorView::cSceneEntity::cSceneEntity(cEditorView * pOuter)
 : m_pOuter(pOuter),
   m_translation(0,0,0),
   m_rotation(0,0,0,1),
   m_transform(tMatrix4::GetIdentity())
{
}

////////////////////////////////////////

cEditorView::cSceneEntity::~cSceneEntity()
{
}

////////////////////////////////////////

void cEditorView::cSceneEntity::Render(IRenderDevice * pRenderDevice)
{
   Assert(m_pOuter != NULL);

   cAutoIPtr<IEditorModel> pModel;
   if (m_pOuter->GetModel(&pModel) == S_OK)
   {
      if (pModel->AccessTerrain() != NULL)
      {
         pModel->AccessTerrain()->Render(pRenderDevice);
      }
   }

   if ((m_pOuter->m_highlitTileX != -1) && (m_pOuter->m_highlitTileZ != -1))
   {
      tVec3 verts[4];
      if (pModel->AccessTerrain()->GetTileVertices(m_pOuter->m_highlitTileX, m_pOuter->m_highlitTileZ, verts) == S_OK)
      {
         static const float kOffsetY = 0.5f;
         verts[0].y += kOffsetY;
         verts[1].y += kOffsetY;
         verts[2].y += kOffsetY;
         verts[3].y += kOffsetY;

         static const GLfloat highlitTileColor[] = { 0,1,0,.25f };

         glPushAttrib(GL_ENABLE_BIT);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBegin(GL_QUADS);
            glColor4fv(highlitTileColor);
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

////////////////////////////////////////

void cEditorView::InitialUpdate() 
{
   cAutoIPtr<IEditorModel> pModel;
   if (GetModel(&pModel) != S_OK || pModel->AccessTerrain() == NULL)
   {
      return;
   }

   uint xDim, zDim;
   pModel->AccessTerrain()->GetDimensions(&xDim, &zDim);

   uint xExt, zExt;
   pModel->AccessTerrain()->GetExtents(&xExt, &zExt);

   PlaceCamera((float)xExt / 2, (float)zExt / 2);

   Update();
}

////////////////////////////////////////

void cEditorView::Update() 
{
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

   return 0;
}
