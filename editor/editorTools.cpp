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
// CLASS: cTerrainTool
//

////////////////////////////////////////

cTerrainTool::cTerrainTool()
{
}

////////////////////////////////////////

cTerrainTool::~cTerrainTool()
{
}

////////////////////////////////////////

bool cTerrainTool::GetHitQuad(CPoint point, IEditorView * pView, HTERRAINQUAD * phQuad)
{
   float ndx, ndy;
   ScreenToNormalizedDeviceCoords(point.x, point.y, &ndx, &ndy);

   if (pView != NULL)
   {
      cRay pickRay;
      if (pView->GeneratePickRay(ndx, ndy, &pickRay) == S_OK)
      {
         UseGlobal(TerrainModel);
         return (pTerrainModel->GetQuadFromHitTest(pickRay, phQuad) == S_OK);
      }
   }

   return false;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainTileTool
//

////////////////////////////////////////

cTerrainTileTool::cTerrainTileTool()
 : m_tile(0)
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
      pView->ClearHighlight();
   }

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      HTERRAINQUAD hQuad = NULL;
      if (GetHitQuad(mouseEvent.GetPoint(), pView, &hQuad))
      {
         pView->HighlightTerrainQuad(hQuad);
      }
      else
      {
         pView->ClearHighlight();
      }
   }

   return cDragTool::OnMouseMove(mouseEvent, pView);
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragStart(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   UseGlobal(TerrainRenderer);
   pTerrainRenderer->EnableBlending(false);

   m_hitQuads.clear();

   Assert(!m_pCommand);
   if (EditorCompositeCommandCreate(&m_pCommand) != S_OK)
   {
      ErrorMsg("Error creating composite command\n");
      return E_FAIL;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragEnd(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   // Do what is done on a move
   tResult result = OnDragMove(mouseEvent, pView);

   UseGlobal(TerrainRenderer);
   pTerrainRenderer->EnableBlending(true);

   Assert(!!m_pCommand);
   cAutoIPtr<IEditorModel> pEditorModel;
   if (pView->GetModel(&pEditorModel) == S_OK)
   {
      pEditorModel->AddCommand(m_pCommand, false);
   }
   SafeRelease(m_pCommand);

   m_hitQuads.clear();

   return result;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnDragMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   Assert(!!m_pCommand);

   HTERRAINQUAD hQuad = NULL;
   if (GetHitQuad(mouseEvent.GetPoint(), pView, &hQuad)
      && m_hitQuads.find(hQuad) == m_hitQuads.end())
   {
      m_hitQuads.insert(hQuad);

      cAutoIPtr<IEditorCommand> pCommand(static_cast<IEditorCommand*>(
         new cTerrainTileCommand(hQuad, m_tile)));

      if (!pCommand)
      {
         return E_OUTOFMEMORY;
      }

      if (pCommand->Do() != S_OK)
      {
         return E_FAIL;
      }

      m_pCommand->Add(pCommand);
   }

   return S_EDITOR_TOOL_HANDLED;
}

/////////////////////////////////////////////////////////////////////////////
