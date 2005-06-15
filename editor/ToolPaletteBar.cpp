/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "editorCtrlBars.h"

#include "globalobj.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int kToolPaletteId = 1234;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

AUTO_REGISTER_CONTROLBAR(IDS_TOOL_PALETTE_BAR_TITLE, RUNTIME_CLASS(cToolPaletteBar), kCBP_Right);

IMPLEMENT_DYNCREATE(cToolPaletteBar, cEditorControlBar);

cToolPaletteBar::cToolPaletteBar()
{
}

cToolPaletteBar::~cToolPaletteBar()
{
}

BEGIN_MESSAGE_MAP(cToolPaletteBar, cEditorControlBar)
   //{{AFX_MSG_MAP(cToolPaletteBar)
   ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
   ON_NOTIFY(kTPN_ItemCheck, kToolPaletteId, OnToolPaletteItemCheck)
   ON_NOTIFY(kTPN_ItemDestroy, kToolPaletteId, OnToolPaletteItemDestroy)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void cToolPaletteBar::OnDefaultTileSetChange(IEditorTileSet * pTileSet)
{
   UseGlobal(EditorTileManager);

   uint nTileSets = 0;
   if (pEditorTileManager->GetTileSetCount(&nTileSets) == S_OK && nTileSets > 0)
   {
      if (m_toolPalette.IsWindow())
      {
         tToolGroups::iterator iter = m_terrainTileGroups.begin();
         tToolGroups::iterator end = m_terrainTileGroups.end();
         for (; iter != end; iter++)
         {
            m_toolPalette.RemoveGroup(*iter);
         }
         m_terrainTileGroups.clear();
      }
      else
      {
         return;
      }

      for (uint i = 0; i < nTileSets; i++)
      {
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (pEditorTileManager->GetTileSet(i, &pTileSet) == S_OK)
         {
            uint nTiles = 0;
            if (pTileSet->GetTileCount(&nTiles) == S_OK && nTiles > 0)
            {
               cStr tileSetName;
               HIMAGELIST hTileSetImages = NULL;
               if (pTileSet->GetName(&tileSetName) == S_OK
                  && pTileSet->GetImageList(16, &hTileSetImages) == S_OK)
               {
                  HTOOLGROUP hGroup = m_toolPalette.AddGroup(tileSetName.c_str(),
                     hTileSetImages);

                  if (hGroup != NULL)
                  {
                     m_terrainTileGroups.push_back(hGroup);

                     for (uint j = 0; j < nTiles; j++)
                     {
                        cStr tileName;
                        if (pTileSet->GetTileName(j, &tileName) == S_OK)
                        {
                           cTerrainTileTool * pTerrainTool = new cTerrainTileTool;
                           if (pTerrainTool != NULL)
                           {
                              pTerrainTool->SetTile(j);
                              m_toolPalette.AddTool(hGroup, tileName.c_str(), j, pTerrainTool);
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
// cToolPaletteBar message handlers

int cToolPaletteBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (cEditorControlBar::OnCreate(lpCreateStruct) == -1)
      return -1;

   if (!m_tooltip.Create(this))
   {
      DebugMsg1("Unable to create tooltip control (error %d)\n", GetLastError());
      return -1;
   }

   if (!m_toolPalette.Create(m_hWnd, CWindow::rcDefault, "", 0, 0, kToolPaletteId))
   {
      ErrorMsg1("Unable to create tool palette control (error %d)\n", GetLastError());
      return -1;
   }

   HIMAGELIST hStdImages = ImageList_LoadImage(_Module.GetResourceInstance(),
      MAKEINTRESOURCE(IDB_STD_TOOLS), 16, 0, CLR_DEFAULT, IMAGE_BITMAP, 0);

   // TODO: Come up with a better way for creating the standard tools
   if (hStdImages != NULL)
   {
      HTOOLGROUP hStdGroup = m_toolPalette.AddGroup("", hStdImages);
      if (hStdGroup != NULL)
      {
         HTOOLITEM hTool = m_toolPalette.AddTool(hStdGroup, "Select", 0, new cMoveCameraTool);
         if (hTool != NULL)
         {
         }
      }
   }

   UseGlobal(EditorTileManager);
   pEditorTileManager->Connect(this);

   cAutoIPtr<IEditorTileSet> pTileSet;
   if (pEditorTileManager->GetDefaultTileSet(&pTileSet) == S_OK)
   {
      OnDefaultTileSetChange(pTileSet);
   }

   return 0;
}

void cToolPaletteBar::OnDestroy() 
{
   cEditorControlBar::OnDestroy();

   UseGlobal(EditorTileManager);
   pEditorTileManager->Disconnect(this);

   m_toolPalette.Clear();
}

void cToolPaletteBar::OnSize(UINT nType, int cx, int cy) 
{
	cEditorControlBar::OnSize(nType, cx, cy);
   if (m_toolPalette.IsWindow())
   {
      CRect rect;
      GetClientRect(rect);
      m_toolPalette.MoveWindow(rect);
   }
}

void cToolPaletteBar::OnToolPaletteItemCheck(NMHDR * pnmh, LRESULT * pResult)
{
   Assert(pnmh != NULL);

   sToolPaletteItem tpi;
   if (m_toolPalette.GetTool(((sNMToolPaletteItemClick *)pnmh)->hTool, &tpi))
   {
      cAutoIPtr<IEditorTool> pEditorTool;
      if (tpi.pUserData != NULL)
      {
         IUnknown * pUnk = reinterpret_cast<IUnknown *>(tpi.pUserData);
         if (pUnk->QueryInterface(&pEditorTool) == S_OK)
         {
            UseGlobal(EditorApp);
            pEditorApp->SetActiveTool(pEditorTool);
         }
      }

      if (!pEditorTool)
      {
         UseGlobal(EditorApp);
         pEditorApp->SetActiveTool(NULL);
      }
   }
}

void cToolPaletteBar::OnToolPaletteItemDestroy(NMHDR * pnmh, LRESULT * pResult)
{
   Assert(pnmh != NULL);

   sToolPaletteItem tpi;
   if (m_toolPalette.GetTool(((sNMToolPaletteItemClick *)pnmh)->hTool, &tpi))
   {
      DebugMsg1("Tool \"%s\" destroyed\n", tpi.szName);
      if (tpi.pUserData != NULL)
      {
         IUnknown * pUnk = reinterpret_cast<IUnknown *>(tpi.pUserData);
         pUnk->Release();
      }
   }
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

   return cEditorControlBar::PreTranslateMessage(pMsg);
}
