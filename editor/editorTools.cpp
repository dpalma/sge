/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTools.h"
#include "editorTypes.h"
#include "editorCommands.h"
#include "terrain.h"

#include "sceneapi.h"
#include "ray.h"

#include "matrix4.h"
#include "vec3.h"
#include "vec4.h"

#include <GL/gl.h>

#include "dbgalloc.h" // must be last header

LOG_DEFINE_CHANNEL(EditorTools);
#define LocalMsg(msg)            DebugMsgEx(EditorTools,(msg))
#define LocalMsg1(msg,a)         DebugMsgEx1(EditorTools,(msg),(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(EditorTools,(msg),(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(EditorTools,(msg),(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(EditorTools,(msg),(a),(b),(c),(d))

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDragTool
//

////////////////////////////////////////

cDragTool::cDragTool()
 : m_nextStamp(0)
{
}

////////////////////////////////////////

cDragTool::~cDragTool()
{
}

////////////////////////////////////////

tResult cDragTool::OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView)
{
   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cDragTool::OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      m_pView = CTAddRef(pView);
      AccessEditorApp()->SetToolCapture(this);
      return OnDragStart(mouseEvent, pView);
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cDragTool::OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (IsDragging())
   {
      tResult result = OnDragEnd(mouseEvent, pView);
      SafeRelease(m_pView);
      AccessEditorApp()->ReleaseToolCapture();
      return result;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cDragTool::OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (IsDragging())
   {
      return OnDragMove(mouseEvent, pView);
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

bool cDragTool::IsDragging()
{
   return (!!m_pView);
}

////////////////////////////////////////

IEditorView * cDragTool::AccessView()
{
   return m_pView;
}

////////////////////////////////////////

ulong cDragTool::GetNextStamp()
{
   return ++m_nextStamp;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMoveCameraTool
//

////////////////////////////////////////

cMoveCameraTool::cMoveCameraTool()
{
}

////////////////////////////////////////

cMoveCameraTool::~cMoveCameraTool()
{
}

////////////////////////////////////////

tResult cMoveCameraTool::OnKeyDown(const cEditorKeyEvent & keyEvent, IEditorView * pView)
{
   static const int kMove = 5;

   switch (keyEvent.GetChar())
   {
      case VK_LEFT:
      {
         MoveCamera(pView, CPoint(-kMove,0));
         break;
      }
      case VK_RIGHT:
      {
         MoveCamera(pView, CPoint(kMove,0));
         break;
      }
      case VK_UP:
      {
         MoveCamera(pView, CPoint(0,-kMove));
         break;
      }
      case VK_DOWN:
      {
         MoveCamera(pView, CPoint(0,kMove));
         break;
      }
   }

   return cDragTool::OnKeyDown(keyEvent, pView);
}

////////////////////////////////////////

tResult cMoveCameraTool::OnMouseWheel(const cEditorMouseWheelEvent & mouseWheelEvent, IEditorView * pView)
{
   static const float kMinElevation = 10;
   static const float kMaxElevation = 500;

   if (pView != NULL)
   {
      float elevation;
      if (pView->GetCameraElevation(&elevation) == S_OK)
      {
         elevation += (mouseWheelEvent.GetZDelta() / WHEEL_DELTA);
         if (elevation >= kMinElevation && elevation <= kMaxElevation)
         {
            pView->SetCameraElevation(elevation);
         }
      }
   }

   return cDragTool::OnMouseWheel(mouseWheelEvent, pView);
}

////////////////////////////////////////

tResult cMoveCameraTool::OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   m_lastMousePoint = mouseEvent.GetPoint();
   return S_EDITOR_TOOL_HANDLED;
}

////////////////////////////////////////

tResult cMoveCameraTool::OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if ((pView != NULL) && CTIsSameObject(pView, AccessView()))
   {
      float camPlaceX, camPlaceZ;
      if ((pView != NULL) && pView->GetCameraPlacement(&camPlaceX, &camPlaceZ) == S_OK)
      {
         techlog.Print(kInfo, "Looking at point (%.2f, 0, %.2f)\n", camPlaceX, camPlaceZ);
      }

      return S_EDITOR_TOOL_HANDLED;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cMoveCameraTool::OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if ((pView != NULL) && CTIsSameObject(pView, AccessView()))
   {
      CPoint delta = mouseEvent.GetPoint() - m_lastMousePoint;
      MoveCamera(pView, delta);
      m_lastMousePoint = mouseEvent.GetPoint();
      return S_EDITOR_TOOL_HANDLED;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

void cMoveCameraTool::MoveCamera(IEditorView * pView, CPoint delta)
{
   if (pView != NULL)
   {
      float camPlaceX, camPlaceZ;
      if (pView->GetCameraPlacement(&camPlaceX, &camPlaceZ) == S_OK)
      {
         camPlaceX += delta.x;
         camPlaceZ += delta.y;

         pView->PlaceCamera(camPlaceX, camPlaceZ);
      }
   }
}


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
//
// CLASS: cTerrainTileTool
//

////////////////////////////////////////

cTerrainTileTool::cTerrainTileTool()
 : m_iLastHitX(~0),
   m_iLastHitZ(~0),
   m_tile(0)
{
}

////////////////////////////////////////

cTerrainTileTool::~cTerrainTileTool()
{
}

////////////////////////////////////////

void cTerrainTileTool::SetTile(uint tile)
{
   m_tile = tile;
}

////////////////////////////////////////

tResult cTerrainTileTool::Activate()
{
   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileTool::Deactivate()
{
   cAutoIPtr<IEditorView> pView;
   if (AccessEditorApp()->GetActiveView(&pView) == S_OK)
   {
      pView->ClearTileHighlight();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      uint ix, iz;
      cTerrainTile * pTile;
      if (GetHitTile(mouseEvent.GetPoint(), pView, &ix, &iz, &pTile))
      {
         pView->HighlightTile(ix, iz);
      }
      else
      {
         pView->ClearTileHighlight();
      }
   }

   return cDragTool::OnMouseMove(mouseEvent, pView);
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   m_currentStamp = GetNextStamp();
   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   // Do what is done on a move
   tResult result = OnDragMove(mouseEvent, pView);
   m_currentStamp = 0;
   return result;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      uint ix, iz;
      if (GetHitTile(mouseEvent.GetPoint(), pView, &ix, &iz, NULL))
      {
         if ((m_iLastHitX != ix) || (m_iLastHitZ != iz))
         {
            m_iLastHitX = ix;
            m_iLastHitZ = iz;

            cAutoIPtr<IEditorModel> pModel;
            if (pView->GetModel(&pModel) == S_OK)
            {
               cAutoIPtr<IEditorCommand> pCommand(new cTerrainTileCommand(
                  pModel->AccessTerrain(), ix, iz, m_tile, m_currentStamp));
               if (!!pCommand)
               {
                  pModel->AddCommand(pCommand);
               }
            }
         }
      }
   }

   return S_EDITOR_TOOL_HANDLED;
}

////////////////////////////////////////

bool cTerrainTileTool::GetHitTile(CPoint point, IEditorView * pView, uint * pix, uint * piz, cTerrainTile * * ppTile)
{
   cAutoIPtr<IEditorModel> pModel;
   if (pView->GetModel(&pModel) == S_OK)
   {
      cAutoIPtr<ISceneCamera> pCamera;
      if (pView->GetCamera(&pCamera) == S_OK)
      {
         float ndx, ndy;
         ScreenToNormalizedDeviceCoords(point.x, point.y, &ndx, &ndy);

         tVec3 pickDir;
         if (GetPickVector(pCamera, ndx, ndy, &pickDir))
         {
            cRay pickRay(pView->GetCameraEyePosition(), pickDir);

            tVec3 pointOnPlane;
            if (pickRay.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
            {
               LocalMsg3("Hit the ground at approximately (%.1f, %.1f, %.1f)\n",
                  pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

               cTerrain * pTerrain = pModel->AccessTerrain();
               if (pTerrain != NULL)
               {
                  uint ix, iz;
                  pTerrain->GetTileIndices(pointOnPlane.x, pointOnPlane.z, &ix, &iz);

                  LocalMsg2("Hit tile (%d, %d)\n", ix, iz);

                  if (pix != NULL)
                  {
                     *pix = ix;
                  }

                  if (piz != NULL)
                  {
                     *piz = iz;
                  }

                  if (ppTile != NULL)
                  {
                     *ppTile = pTerrain->GetTile(ix, iz);
                  }

                  return true;
               }
            }
         }
      }
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////
