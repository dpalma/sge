/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "editorCtrlBars.h"
#include "terrain.h"

#include "sceneapi.h"
#include "ray.h"

#include "globalobj.h"
#include "matrix4.h"
#include "vec3.h"
#include "vec4.h"

#include "resource.h"       // main symbols

#include <GL/gl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const uint kMaxButtons = 100; // arbitrary - used only for ON_CONTROL_RANGE
static const uint kButtonSize = 32;
static const uint kButtonFirstId = 1000;
static const CRect buttonMargins(5,5,5,5);

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
// CLASS: cButtonPanel
//

////////////////////////////////////////

cButtonPanel::cButtonPanel()
 : m_margins(buttonMargins),
   m_pTool(new cTerrainTileTool)
{
}

////////////////////////////////////////

cButtonPanel::~cButtonPanel()
{
   Assert(m_buttons.empty());
}

////////////////////////////////////////

void cButtonPanel::AddButton(CButton * pButton)
{
   ASSERT_VALID(pButton);
   m_buttons.push_back(pButton);
}

////////////////////////////////////////

void cButtonPanel::Clear()
{
   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      (*iter)->DestroyWindow();
      delete *iter;
   }
   m_buttons.clear();
}

////////////////////////////////////////

void cButtonPanel::Reposition(LPCRECT pRect, BOOL bRepaint)
{
   CRect buttonRect;
   buttonRect.left = pRect->left + m_margins.left;
   buttonRect.top = pRect->top + m_margins.top;
   buttonRect.right = buttonRect.left + kButtonSize;
   buttonRect.bottom = buttonRect.top + kButtonSize;

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->GetSafeHwnd()))
      {
         (*iter)->MoveWindow(buttonRect, bRepaint);

         buttonRect.OffsetRect(buttonRect.Width(), 0);

         if (buttonRect.left > (pRect->right - m_margins.right)
            || buttonRect.right > (pRect->right - m_margins.right))
         {
            buttonRect.left = pRect->left + m_margins.left;
            buttonRect.top += kButtonSize;;
            buttonRect.right = buttonRect.left + kButtonSize;
            buttonRect.bottom = buttonRect.top + kButtonSize;
         }
      }
   }
}

////////////////////////////////////////

void cButtonPanel::HandleClick(uint buttonId)
{
   Assert(!!m_pTool);

   AccessEditorApp()->SetActiveTool(m_pTool);

   m_pTool->SetTile(buttonId - m_buttons[0]->GetDlgCtrlID());

   CButton * pClickedButton = NULL;

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->GetSafeHwnd()) && (*iter)->GetDlgCtrlID() != buttonId)
      {
         (*iter)->SendMessage(BM_SETSTATE, FALSE);
      }

      if ((*iter)->GetDlgCtrlID() == buttonId)
      {
         pClickedButton = *iter;
      }
   }

   ASSERT_VALID(pClickedButton);
   pClickedButton->SendMessage(BM_SETSTATE, TRUE);
}

////////////////////////////////////////

void cButtonPanel::SetMargins(LPCRECT pMargins)
{
   if (pMargins != NULL)
   {
      m_margins = *pMargins;
   }
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

   cAutoIPtr<IEditorModel> pModel;
   if (pView->GetModel(&pModel) == S_OK)
   {
      cAutoIPtr<ISceneCamera> pCamera;
      if (pView->GetCamera(&pCamera) == S_OK)
      {
         float ndx, ndy;
         ScreenToNormalizedDeviceCoords(mouseEvent.GetPoint().x,
                                        mouseEvent.GetPoint().y,
                                        &ndx, &ndy);

         tVec3 pickDir;
         if (GetPickVector(pCamera, ndx, ndy, &pickDir))
         {
            cRay pickRay(pView->GetCameraEyePosition(), pickDir);

            tVec3 pointOnPlane;
            if (pickRay.IntersectsPlane(tVec3(0,1,0), 0, &pointOnPlane))
            {
               DebugMsg3("Hit the ground at approximately (%.1f, %.1f, %.1f)\n",
                  pointOnPlane.x, pointOnPlane.y, pointOnPlane.z);

               cTerrain * pTerrain = pModel->AccessTerrain();
               if (pTerrain != NULL)
               {
                  uint mapDimX, mapDimZ, mapExtX, mapExtZ;
                  pTerrain->GetDimensions(&mapDimX, &mapDimZ);
                  pTerrain->GetExtents(&mapExtX, &mapExtZ);

                  uint tileWidth = mapExtX / mapDimX;
                  uint tileDepth = mapExtZ / mapDimZ;

                  int iTileX = Round(pointOnPlane.x / tileWidth);
                  int iTileZ = Round(pointOnPlane.z / tileDepth);

                  cTerrainTile * pTile = pTerrain->GetTile(iTileX, iTileZ);
                  pTile->SetTile(m_tile);

                  DebugMsg2("Hit tile (%d, %d)\n", iTileX, iTileZ);
               }
            }
         }
      }
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
   return S_EDITOR_TOOL_CONTINUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

AUTO_REGISTER_CONTROLBAR(IDS_TOOL_PALETTE_BAR_TITLE, RUNTIME_CLASS(cToolPaletteBar), kCBP_Right);

IMPLEMENT_DYNCREATE(cToolPaletteBar, CSizingControlBarG);

cToolPaletteBar::cToolPaletteBar()
{
}

cToolPaletteBar::~cToolPaletteBar()
{
}

BEGIN_MESSAGE_MAP(cToolPaletteBar, CSizingControlBarG)
   //{{AFX_MSG_MAP(cToolPaletteBar)
   ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
   ON_CONTROL_RANGE(BN_CLICKED, kButtonFirstId, kButtonFirstId+kMaxButtons, OnButtonClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void cToolPaletteBar::OnDefaultTileSetChange(IEditorTileSet * pTileSet)
{
   ClearButtons();

   if (pTileSet != NULL)
   {
      uint nTiles = 0;
      if (pTileSet->GetTileCount(&nTiles) == S_OK)
      {
         for (uint i = 0; i < nTiles; i++)
         {
            cAutoIPtr<IEditorTile> pTile;
            if (pTileSet->GetTile(i, &pTile) == S_OK)
            {
               cStr tileName;
               HBITMAP hBitmap = NULL;
               if (pTile->GetName(&tileName) == S_OK
                  && pTile->GetBitmap(kButtonSize, false, &hBitmap) == S_OK)
               {
                  CButton * pButton = new CButton();
                  if (pButton != NULL)
                  {
                     if (pButton->Create(NULL, WS_CHILD | WS_VISIBLE | BS_BITMAP,
                        CRect(CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT),
                        this, kButtonFirstId + i))
                     {
                        pButton->SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

                        m_tooltip.AddTool(pButton, tileName.c_str());

                        m_buttonPanel.AddButton(pButton);
                     }
                     else
                     {
                        delete pButton;
                     }
                  }
               }
            }
         }
      }

      RepositionButtons();
   }
}

void cToolPaletteBar::ClearButtons()
{
   m_buttonPanel.Clear();
}

void cToolPaletteBar::RepositionButtons(BOOL bRepaint)
{
   CRect rect;
   GetClientRect(rect);

   m_buttonPanel.Reposition(rect, bRepaint);
}

/////////////////////////////////////////////////////////////////////////////
// cToolPaletteBar message handlers

int cToolPaletteBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
      return -1;

   if (!m_tooltip.Create(this))
   {
      DebugMsg1("Unable to create tooltip control (error %d)\n", GetLastError());
      return -1;
   }

   UseGlobal(EditorTileManager);
   pEditorTileManager->Connect(this);

   return 0;
}

void cToolPaletteBar::OnDestroy() 
{
   CSizingControlBarG::OnDestroy();

   UseGlobal(EditorTileManager);
   pEditorTileManager->Disconnect(this);

   ClearButtons();
}

void cToolPaletteBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBarG::OnSize(nType, cx, cy);
	
   RepositionButtons();
}

void cToolPaletteBar::OnButtonClicked(uint buttonId)
{
   m_buttonPanel.HandleClick(buttonId);
}

BOOL cToolPaletteBar::PreTranslateMessage(MSG* pMsg) 
{
   if (IsWindow(m_tooltip.GetSafeHwnd()))
   {
      if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
      {
         m_tooltip.RelayEvent(pMsg);
      }
   }

   return CSizingControlBarG::PreTranslateMessage(pMsg);
}
