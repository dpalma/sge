/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorView.h"
#include "editorDoc.h"
#include "terrain.h"
#include "editorapi.h"
#include "editorTools.h"

#include "sceneapi.h"

#include "renderapi.h"

#include "globalobj.h"

#include <algorithm>
#include <GL/gl.h>
#include <zmouse.h>

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////

const uint WM_GET_IEDITORVIEW = RegisterWindowMessage("WM_GET_IEDITORVIEW");

const float kFov = 70;
const float kZNear = 1;
const float kZFar = 5000;

static const GLfloat kHighlightTileColor[] = { 0, 1, 0, 0.25f };

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
   Verify(SetModel(NULL) == S_OK);
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
   ClearTileHighlight();
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

      cAutoIPtr<IEditorModel> pModel;
      if (GetModel(&pModel) == S_OK
         && pModel->AccessTerrain() != NULL)
      {
         int iHlx, iHlz;
         if (GetHighlightTile(&iHlx, &iHlz) == S_OK)
         {
            tVec3 verts[4];
            if (pModel->AccessTerrain()->GetTileVertices(iHlx, iHlz, verts) == S_OK)
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

   UseGlobal(EditorApp);
   Verify(pEditorApp->AddLoopClient(this) == S_OK);

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
   UseGlobal(EditorApp);
   pEditorApp->RemoveLoopClient(this);

   UseGlobal(Scene);
   pScene->Clear(kSL_Terrain);

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

   std::vector<ISceneEntity *> entities;
   if (pModel->AccessTerrain()->GetSceneEntities(&entities) == S_OK)
   {
      UseGlobal(Scene);
      std::vector<ISceneEntity *>::iterator iter = entities.begin();
      std::vector<ISceneEntity *>::iterator end = entities.end();
      for (; iter != end; iter++)
      {
         pScene->AddEntity(kSL_Terrain, *iter);
      }
   }
   std::for_each(entities.begin(), entities.end(), CTInterfaceMethod(&ISceneEntity::Release));
}

/////////////////////////////////////////////////////////////////////////////
