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

IMPLEMENT_DYNCREATE(cEditorView, CView)

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
   m_highlitTileZ(-1),
   m_bInPostNcDestroy(false)
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

   UseGlobal(EditorApp);
   Verify(pEditorApp->AddLoopClient(this) == S_OK);

	return 0;
}

////////////////////////////////////////

void cEditorView::OnDestroy() 
{
	cGLView::OnDestroy();

   UseGlobal(EditorApp);
   pEditorApp->RemoveLoopClient(this);

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
