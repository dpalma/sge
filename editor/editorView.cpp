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

cEditorView::cEditorView()
 : m_pDocument(NULL),
   m_cameraElevation(kDefaultCameraElevation),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_bRecalcEye(true),
   m_nIndices(0),
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

   SafeRelease(m_pVertexBuffer);
   SafeRelease(m_pIndexBuffer);
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

	cEditorDoc * pDoc = m_pOuter->GetDocument();
	if (pDoc == NULL)
   {
      return;
   }

   tResult renderResult = S_FALSE;

   if (pDoc->AccessTerrain() != NULL)
   {
      renderResult = pDoc->AccessTerrain()->Render(pRenderDevice);
   }

   if (renderResult != S_OK)
   {
      pRenderDevice->Render(
         kRP_Triangles, 
         pDoc->AccessMaterial(), 
         m_pOuter->m_nIndices, 
         m_pOuter->m_pIndexBuffer,
         0, 
         m_pOuter->m_pVertexBuffer);
   }

   if ((m_pOuter->m_highlitTileX != -1) && (m_pOuter->m_highlitTileZ != -1))
   {
      cTerrainTile * pTile = pDoc->AccessTerrain()->GetTile(m_pOuter->m_highlitTileX, m_pOuter->m_highlitTileZ);
      if (pTile != NULL)
      {
         sTerrainVertex verts[4];
         memcpy(verts, pTile->GetVertices(), 4 * sizeof(sTerrainVertex));

         static const float kOffsetY = 0.5f;

         verts[0].pos.y += kOffsetY;
         verts[1].pos.y += kOffsetY;
         verts[2].pos.y += kOffsetY;
         verts[3].pos.y += kOffsetY;

         glPushAttrib(GL_ENABLE_BIT);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glBegin(GL_QUADS);
            glColor4f(0, 1, 0, 0.25f);
            glNormal3f(0, 1, 0);
            glVertex3fv(verts[0].pos.v);
            glVertex3fv(verts[3].pos.v);
            glVertex3fv(verts[2].pos.v);
            glVertex3fv(verts[1].pos.v);
         glEnd();
         glPopAttrib();
      }
   }
}

////////////////////////////////////////

void cEditorView::InitialUpdate() 
{
   SafeRelease(m_pVertexBuffer);
   SafeRelease(m_pIndexBuffer);

   CRect rect;
   GetClientRect(rect);

	cEditorDoc * pDoc = GetDocument();
   if (pDoc == NULL || pDoc->AccessTerrain() == NULL)
   {
      return;
   }

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (TerrainVertexDeclarationCreate(AccessRenderDevice(), &pVertexDecl) == S_OK)
   {
      if (AccessRenderDevice()->CreateVertexBuffer(pDoc->GetVertexCount(),
         kBU_Default, pVertexDecl, kBP_Auto, &m_pVertexBuffer) == S_OK)
      {
         void * pVertexData = NULL;
         if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
         {
            memset(pVertexData, 0, pDoc->GetVertexCount() * sizeof(sTerrainVertex));
            m_pVertexBuffer->Unlock();
         }
      }
   }

   uint xDim, zDim;
   pDoc->AccessTerrain()->GetDimensions(&xDim, &zDim);

   m_nIndices = xDim * zDim * 6;

   if (AccessRenderDevice()->CreateIndexBuffer(m_nIndices,
      kBU_Default, kIBF_16Bit, kBP_System, &m_pIndexBuffer) == S_OK)
   {
      void * pIndexData = NULL;
      if (m_pIndexBuffer->Lock(kBL_Discard, (void * *)&pIndexData) == S_OK)
      {
         memset(pIndexData, 0, m_nIndices * sizeof(uint16));
         m_pIndexBuffer->Unlock();
      }
   }

   uint xExt, zExt;
   pDoc->AccessTerrain()->GetExtents(&xExt, &zExt);

   PlaceCamera((float)xExt / 2, (float)zExt / 2);
}

////////////////////////////////////////

void cEditorView::Update() 
{
	cEditorDoc * pDoc = GetDocument();
	Assert(pDoc != NULL);

   if (!!m_pVertexBuffer)
   {
      void * pVertexData = NULL;
      if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
      {
         memcpy(pVertexData, pDoc->GetVertexPointer(),
            pDoc->GetVertexCount() * sizeof(sTerrainVertex));
         m_pVertexBuffer->Unlock();
      }
   }

   if (!!m_pIndexBuffer)
   {
      uint16 * pIndexData = NULL;
      if (m_pIndexBuffer->Lock(kBL_Discard, (void * *)&pIndexData) == S_OK)
      {
         int iQuad = 0;

         uint xDim, zDim;
         pDoc->AccessTerrain()->GetDimensions(&xDim, &zDim);

         for (uint iz = 0; iz < zDim; iz++)
         {
            for (uint ix = 0; ix < xDim; ix++, iQuad++)
            {
               pIndexData[(iQuad * 6) + 0] = (iQuad * 4) + 0;
               pIndexData[(iQuad * 6) + 1] = (iQuad * 4) + 3;
               pIndexData[(iQuad * 6) + 2] = (iQuad * 4) + 2;

               pIndexData[(iQuad * 6) + 3] = (iQuad * 4) + 2;
               pIndexData[(iQuad * 6) + 4] = (iQuad * 4) + 1;
               pIndexData[(iQuad * 6) + 5] = (iQuad * 4) + 0;
            }
         }

         m_pIndexBuffer->Unlock();
      }
   }
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
