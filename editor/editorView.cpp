/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorApp.h"
#include "editorDoc.h"
#include "editorView.h"

#include "sceneapi.h"

#include "render.h"

#include "globalobj.h"
#include "matrix4.h"
#include "vec2.h"
#include "keys.h"
#include "techtime.h"

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const GLfloat kFov = 70;
const GLfloat kZNear = 1;
const GLfloat kZFar = 5000;

/////////////////////////////////////////////////////////////////////////////
// cEditorView

IMPLEMENT_DYNCREATE(cEditorView, CView)

BEGIN_MESSAGE_MAP(cEditorView, cGLView)
	//{{AFX_MSG_MAP(cEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorView construction/destruction

cEditorView::cEditorView()
 : m_center(0,0,0),
   m_eye(0,0,0)
{
}

cEditorView::~cEditorView()
{
}

/////////////////////////////////////////////////////////////////////////////
// cEditorView operations

tResult cEditorView::Create(int width, int height, int bpp, const char * pszTitle)
{
   Assert(!"This should never be called");
   return E_FAIL;
}

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

tResult cEditorView::SwapBuffers()
{
   return ::SwapBuffers(GetSafeHdc()) ? S_OK : E_FAIL;
}

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

}

/////////////////////////////////////////////////////////////////////////////
// cEditorView diagnostics

#ifdef _DEBUG
void cEditorView::AssertValid() const
{
	cGLView::AssertValid();
}

void cEditorView::Dump(CDumpContext& dc) const
{
	cGLView::Dump(dc);
}

cEditorDoc* cEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(cEditorDoc)));
	return (cEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cEditorView message handlers

int cEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (cGLView::OnCreate(lpCreateStruct) == -1)
		return -1;

   if (RenderDeviceCreate(0, &m_pRenderDevice) != S_OK)
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

	return 0;
}

void cEditorView::OnDestroy() 
{
	cGLView::OnDestroy();

   Assert(AccessEditorApp() != NULL);
   Verify(AccessEditorApp()->RemoveLoopClient(this) == S_OK);

   SafeRelease(m_pCamera);
   SafeRelease(m_pRenderDevice);
}

void cEditorView::OnSize(UINT nType, int cx, int cy) 
{
	cGLView::OnSize(nType, cx, cy);

   GLfloat aspect = (GLfloat)cx / cy;

   if (AccessRenderDevice() != NULL)
   {
      AccessRenderDevice()->SetViewportSize(cx, cy);
   }

   if (!!m_pCamera)
   {
      m_pCamera->SetPerspective(kFov, aspect, kZNear, kZFar);
   }
}

static tVec3 CalcEyePoint(const tVec3 & lookAt,
                          tVec3::value_type elevation = 100,
                          tVec3::value_type pitch = 70)
{
   return lookAt + tVec3(0, elevation, elevation / tanf(pitch));
}

void cEditorView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	cEditorDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   Assert(!m_pTerrainRoot);
   m_pTerrainRoot = TerrainNodeCreate(m_pRenderDevice, "ground.tga", 0.25, "grass.tga");
   if (!!m_pTerrainRoot)
   {
      UseGlobal(Scene);
      pScene->AddEntity(kSL_Terrain, m_pTerrainRoot);

      tVec2 groundDims = m_pTerrainRoot->GetDimensions();

      m_center = tVec3(groundDims.x / 2, 0, groundDims.y / 2);
      m_eye = CalcEyePoint(m_center);
   }
   else
   {
      m_center = tVec3(0,0,0);
      m_eye = tVec3(0,0,0);
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

      case WM_PAINT:
      {
         // Eat paint messages because the view is drawn in a continuous rendering loop
         ValidateRect(NULL);
         return 0;
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
