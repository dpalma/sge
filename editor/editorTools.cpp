/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTools.h"
#include "editorTypes.h"
#include "editorCommands.h"
#include "terrainapi.h"

#include "ray.h"

#include "globalobj.h"

#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

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
      UseGlobal(EditorApp);
      pEditorApp->SetToolCapture(this);
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
      UseGlobal(EditorApp);
      pEditorApp->ReleaseToolCapture();
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
         InfoMsg2("Looking at point (%.2f, 0, %.2f)\n", camPlaceX, camPlaceZ);
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
   UseGlobal(EditorApp);
   cAutoIPtr<IEditorView> pView;
   if (pEditorApp->GetActiveView(&pView) == S_OK)
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
      if (GetHitTile(mouseEvent.GetPoint(), pView, &ix, &iz))
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
   UseGlobal(TerrainRenderer);
   pTerrainRenderer->EnableBlending(false);

   m_currentStamp = GetNextStamp();
   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   // Do what is done on a move
   tResult result = OnDragMove(mouseEvent, pView);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->EnableBlending(true);

   m_currentStamp = 0;
   return result;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      uint ix, iz;
      if (GetHitTile(mouseEvent.GetPoint(), pView, &ix, &iz))
      {
         if ((m_iLastHitX != ix) || (m_iLastHitZ != iz))
         {
            m_iLastHitX = ix;
            m_iLastHitZ = iz;

            UseGlobal(TerrainModel);

            cAutoIPtr<IEditorCommand> pCommand(new cTerrainTileCommand(
               pTerrainModel, ix, iz, m_tile, m_currentStamp));
            if (!!pCommand)
            {
               cAutoIPtr<IEditorModel> pEM;
               if (pView->GetModel(&pEM) == S_OK)
               {
                  pEM->AddCommand(pCommand);
               }
            }
         }
      }
   }

   return S_EDITOR_TOOL_HANDLED;
}

////////////////////////////////////////

bool cTerrainTileTool::GetHitTile(CPoint point, IEditorView * pView, uint * pix, uint * piz)
{
   float ndx, ndy;
   ScreenToNormalizedDeviceCoords(point.x, point.y, &ndx, &ndy);

   cRay pickRay;
   if (pView->GeneratePickRay(ndx, ndy, &pickRay) == S_OK)
   {
      tVec3 pointOnPlane;
      if (pickRay.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
      {
         LocalMsg3("Hit the ground at approximately (%.1f, %.1f, %.1f)\n",
            pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

         UseGlobal(TerrainModel);

         uint ix, iz;
         pTerrainModel->GetTileIndices(pointOnPlane.x, pointOnPlane.z, &ix, &iz);

         LocalMsg2("Hit tile (%d, %d)\n", ix, iz);

         if (pix != NULL)
         {
            *pix = ix;
         }
         if (piz != NULL)
         {
            *piz = iz;
         }
         return true;
      }
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////
