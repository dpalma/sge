/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "editorView.h"

#include "sceneapi.h"
#include "ray.h"

#include "render.h"

#include "globalobj.h"
#include "matrix4.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "keys.h"
#include "techtime.h"

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const float kFov = 70;
const float kZNear = 1;
const float kZFar = 5000;

/////////////////////////////////////////////////////////////////////////////

static void ScreenToNormalizedDeviceCoords(int sx, int sy,
                                           float * pndx, float * pndy)
{
   Assert(pndx != NULL);
   Assert(pndy != NULL);

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   sy = viewport[3] - sy;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(sx - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(sy - viewport[1]) * 2.f / viewport[3] - 1.f;

   *pndx = normx;
   *pndy = normy;
}

/////////////////////////////////////////////////////////////////////////////

static bool GetPickVector(ISceneCamera * pCamera, float ndx, float ndy, tVec3 * pPickDir)
{
   Assert(pCamera != NULL);

   const tMatrix4 & m = pCamera->GetViewProjectionInverseMatrix();

   tVec4 n = m.Transform(tVec4(ndx, ndy, -1, 1));
   if (n.w == 0.0f)
   {
      return false;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f = m.Transform(tVec4(ndx, ndy, 1, 1));
   if (f.w == 0.0f)
   {
      return false;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   if (pPickDir != NULL)
   {
      *pPickDir = tVec3(f.x - n.x, f.y - n.y, f.z - n.z);
      pPickDir->Normalize();
   }

   return true;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView

////////////////////////////////////////

IMPLEMENT_DYNCREATE(cEditorView, CView)

////////////////////////////////////////

BEGIN_MESSAGE_MAP(cEditorView, cGLView)
	//{{AFX_MSG_MAP(cEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

////////////////////////////////////////

cEditorView::cEditorView()
 : m_mouseAction(kNone),
   m_center(0,0,0),
   m_eye(0,0,0),
   m_nIndices(0),
   m_sceneEntity(this)
{
}

////////////////////////////////////////

cEditorView::~cEditorView()
{
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

////////////////////////////////////////

tResult cEditorView::Create(int width, int height, int bpp, const char * pszTitle)
{
   Assert(!"This should never be called");
   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorView::GetWindowInfo(sWindowInfo * pInfo) const
{
   if (pInfo == NULL)
   {
      return E_POINTER;
   }

   if (!IsWindow(GetSafeHwnd()))
   {
      return E_FAIL;
   }

   CRect rect;
   GetClientRect(&rect);

   pInfo->width = rect.Width();
   pInfo->height = rect.Height();
   pInfo->bpp = (GetSafeHdc() != NULL) ? GetDeviceCaps(GetSafeHdc(), BITSPIXEL) : 0;
   pInfo->hWnd = GetSafeHwnd();

   return S_OK;
}

////////////////////////////////////////

tResult cEditorView::SwapBuffers()
{
   return ::SwapBuffers(GetSafeHdc()) ? S_OK : E_FAIL;
}

////////////////////////////////////////

void cEditorView::OnFrame(double time, double elapsed)
{
//   DebugMsg2("cEditorView::OnFrame(%f, %f)\n", time, elapsed);

   if (!!m_pCamera)
   {
      tMatrix4 view;
      MatrixLookAt(m_eye, m_center, tVec3(0,1,0), &view);
      m_pCamera->SetViewMatrix(view);
   }

   RenderScene();
}

////////////////////////////////////////

void cEditorView::RenderScene()
{
   AccessRenderDevice()->BeginScene();
   AccessRenderDevice()->Clear();

   UseGlobal(Scene);
   pScene->Render(AccessRenderDevice());

   AccessRenderDevice()->EndScene();

   SwapBuffers();
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

////////////////////////////////////////

cEditorView::cSceneEntity::cSceneEntity(cEditorView * pOuter)
 : m_pOuter(pOuter),
   m_translation(0,0,0),
   m_rotation(0,0,0,1)
{
   m_transform.Identity();
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
	ASSERT_VALID(pDoc);

   if (pDoc->AccessTerrain() != NULL)
   {
      pDoc->AccessTerrain()->Render(pRenderDevice);
   }

   pRenderDevice->Render(
      kRP_Triangles, 
      pDoc->AccessMaterial(), 
      m_pOuter->m_nIndices, 
      m_pOuter->m_pIndexBuffer,
      0, 
      m_pOuter->m_pVertexBuffer);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

////////////////////////////////////////

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (cGLView::OnCreate(lpCreateStruct) == -1)
		return -1;

   if (RenderDeviceCreate(kRDO_ShowStatistics, &m_pRenderDevice) != S_OK)
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

   pScene->AddEntity(kSL_Terrain, &m_sceneEntity);

   Assert(AccessEditorApp() != NULL);
   Verify(AccessEditorApp()->AddLoopClient(this) == S_OK);

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	cGLView::OnDestroy();

   UseGlobal(Scene);
   pScene->RemoveEntity(kSL_Terrain, &m_sceneEntity);

   Assert(AccessEditorApp() != NULL);
   AccessEditorApp()->RemoveLoopClient(this);

   SafeRelease(m_pCamera);
   SafeRelease(m_pRenderDevice);

   SafeRelease(m_pVertexBuffer);
   SafeRelease(m_pIndexBuffer);
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

static tVec3 CalcEyePoint(const tVec3 & lookAt,
                          tVec3::value_type elevation = 100,
                          tVec3::value_type pitch = 70)
{
   return lookAt + tVec3(0, elevation, elevation / tanf(pitch));
}

////////////////////////////////////////

void cEditorView::OnInitialUpdate() 
{
   SafeRelease(m_pVertexBuffer);
   SafeRelease(m_pIndexBuffer);

	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   cAutoIPtr<IVertexDeclaration> pVertexDecl;
   if (AccessRenderDevice()->CreateVertexDeclaration(g_mapVertexDecl,
      g_nMapVertexMembers, &pVertexDecl) == S_OK)
   {
      if (AccessRenderDevice()->CreateVertexBuffer(pDoc->GetVertexCount(),
         kBU_Default, pVertexDecl, kBP_Auto, &m_pVertexBuffer) == S_OK)
      {
         void * pVertexData = NULL;
         if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
         {
            memset(pVertexData, 0, pDoc->GetVertexCount() * sizeof(sMapVertex));
            m_pVertexBuffer->Unlock();
         }
      }
   }

   uint xDim, zDim;
   pDoc->GetMapDimensions(&xDim, &zDim);

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
   pDoc->GetMapExtents(&xExt, &zExt);

   m_center = tVec3((tVec3::value_type)xExt / 2, 0, (tVec3::value_type)zExt / 2);
   m_eye = CalcEyePoint(m_center);

	CView::OnInitialUpdate();
}

////////////////////////////////////////

void cEditorView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   if (!!m_pVertexBuffer)
   {
      void * pVertexData = NULL;
      if (m_pVertexBuffer->Lock(kBL_Discard, (void * *)&pVertexData) == S_OK)
      {
         memcpy(pVertexData, pDoc->GetVertexPointer(),
            pDoc->GetVertexCount() * sizeof(sMapVertex));
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
         pDoc->GetMapDimensions(&xDim, &zDim);

         for (int iz = 0; iz < zDim; iz++)
         {
            for (int ix = 0; ix < xDim; ix++, iQuad++)
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
	
	return CView::WindowProc(message, wParam, lParam);
}

////////////////////////////////////////

void cEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   float ndx, ndy;
   ScreenToNormalizedDeviceCoords(point.x, point.y, &ndx, &ndy);

   tVec3 pickDir;
   if (GetPickVector(m_pCamera, ndx, ndy, &pickDir))
   {
      cRay pickRay(m_eye, pickDir);

      tVec3 pointOnPlane;
      if (pickRay.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
      {
         DebugMsg3("Hit the ground at approximately (%.1f, %.1f, %.1f)\n",
            pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

         uint mapDimX, mapDimZ, mapExtX, mapExtZ;
         pDoc->GetMapDimensions(&mapDimX, &mapDimZ);
         pDoc->GetMapExtents(&mapExtX, &mapExtZ);

         int iTileX = Round(pointOnPlane.x / mapDimX);
         int iTileZ = Round(pointOnPlane.z / mapDimZ);

         DebugMsg2("Hit tile (%d, %d)\n", iTileX, iTileZ);
      }
   }

	cGLView::OnLButtonDown(nFlags, point);
}

////////////////////////////////////////

void cEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	cGLView::OnLButtonUp(nFlags, point);
}

////////////////////////////////////////

void cEditorView::OnRButtonDown(UINT nFlags, CPoint point) 
{
   SetCapture();
   m_mouseAction = kMoveCamera;
   m_lastMousePoint = point;
	
	cGLView::OnRButtonDown(nFlags, point);
}

////////////////////////////////////////

void cEditorView::OnRButtonUp(UINT nFlags, CPoint point) 
{
   if (m_mouseAction == kMoveCamera)
   {
      techlog.Print(kInfo, "Looking at point (%.2f, 0, %.2f)\n", m_center.x, m_center.z);
   }

   Verify(ReleaseCapture());
   m_mouseAction = kNone;
	
	cGLView::OnRButtonUp(nFlags, point);
}

////////////////////////////////////////

void cEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

   if (GetCapture() == this)
   {
      if (m_mouseAction == kMoveCamera)
      {
         CPoint delta = point - m_lastMousePoint;
         m_eye.x += delta.x;
         m_eye.z += delta.y;
         m_center.x += delta.x;
         m_center.z += delta.y;
      }

      m_lastMousePoint = point;
   }
	
	cGLView::OnMouseMove(nFlags, point);
}
