/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorTools.h"
#include "editorTypes.h"
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

tResult cMoveCameraTool::OnLButtonDown(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      m_pView = CTAddRef(pView);
      m_lastMousePoint = mouseEvent.GetPoint();
      AccessEditorApp()->SetToolCapture(this);
      return S_EDITOR_TOOL_HANDLED;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cMoveCameraTool::OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if ((pView != NULL) && (!!m_pView) && CTIsSameObject(pView, m_pView))
   {
      float camPlaceX, camPlaceZ;
      if ((pView != NULL) && pView->GetCameraPlacement(&camPlaceX, &camPlaceZ) == S_OK)
      {
         techlog.Print(kInfo, "Looking at point (%.2f, 0, %.2f)\n", camPlaceX, camPlaceZ);
      }

      AccessEditorApp()->ReleaseToolCapture();
      SafeRelease(m_pView);

      return S_EDITOR_TOOL_HANDLED;
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cMoveCameraTool::OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if ((pView != NULL) && (!!m_pView) && CTIsSameObject(pView, m_pView))
   {
      CPoint delta = mouseEvent.GetPoint() - m_lastMousePoint;

      float camPlaceX, camPlaceZ;
      if (pView->GetCameraPlacement(&camPlaceX, &camPlaceZ) == S_OK)
      {
         camPlaceX += delta.x;
         camPlaceZ += delta.y;

         pView->PlaceCamera(camPlaceX, camPlaceZ);
      }

      m_lastMousePoint = mouseEvent.GetPoint();

      return S_EDITOR_TOOL_HANDLED;
   }

   return S_EDITOR_TOOL_CONTINUE;
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

tResult cTerrainTileTool::OnLButtonDown(const cEditorMouseEvent & mouseEvent,
                                        IEditorView * pView)
{
   if (pView == NULL)
   {
      return S_EDITOR_TOOL_CONTINUE;
   }

   int ix, iz;
   cTerrainTile * pTile;
   if (GetHitTile(mouseEvent.GetPoint(), pView, &ix, &iz, &pTile))
   {
      pTile->SetTile(m_tile);
   }

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnLButtonUp(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

tResult cTerrainTileTool::OnMouseMove(const cEditorMouseEvent & mouseEvent, IEditorView * pView)
{
   if (pView != NULL)
   {
      int ix, iz;
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

   return S_EDITOR_TOOL_CONTINUE;
}

////////////////////////////////////////

bool cTerrainTileTool::GetHitTile(CPoint point, IEditorView * pView, int * pix, int * piz, cTerrainTile * * ppTile)
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
                  uint mapDimX, mapDimZ, mapExtX, mapExtZ;
                  pTerrain->GetDimensions(&mapDimX, &mapDimZ);
                  pTerrain->GetExtents(&mapExtX, &mapExtZ);

                  uint tileWidth = mapExtX / mapDimX;
                  uint tileDepth = mapExtZ / mapDimZ;

                  int ix = Round(pointOnPlane.x / tileWidth);
                  int iz = Round(pointOnPlane.z / tileDepth);

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
