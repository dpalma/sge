/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "globalobj.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteBar
//

////////////////////////////////////////

AUTO_REGISTER_DOCKINGWINDOW_SIZED(IDS_TOOL_PALETTE_BAR_TITLE, cToolPaletteBar::Factory, kDWP_Right, 150, 0);

////////////////////////////////////////

tResult cToolPaletteBar::Factory(cDockingWindow * * ppDockingWindow)
{
   if (ppDockingWindow == NULL)
   {
      return E_POINTER;
   }
   cToolPaletteBar * pToolPaletteBar = new cToolPaletteBar;
   if (pToolPaletteBar == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *ppDockingWindow = static_cast<cDockingWindow *>(pToolPaletteBar);
   return S_OK;
}

////////////////////////////////////////

cToolPaletteBar::cToolPaletteBar()
{
}

////////////////////////////////////////

cToolPaletteBar::~cToolPaletteBar()
{
}

////////////////////////////////////////

void cToolPaletteBar::OnDefaultTileSetChange(IEditorTileSet * /*pTileSet*/)
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
                  // cToolPalette::AddGroup() expects to take ownership of image
                  // list, but the tileset stores it internally; hence, the
                  // ImageList_Duplicate() call.
                  HTOOLGROUP hGroup = m_toolPalette.AddGroup(tileSetName.c_str(),
                     ImageList_Duplicate(hTileSetImages));

                  if (hGroup != NULL)
                  {
                     m_terrainTileGroups.push_back(hGroup);

                     for (uint j = 0; j < nTiles; j++)
                     {
                        cAutoIPtr<IEditorTile> pTile;
                        if (pTileSet->GetTile(j, &pTile) == S_OK)
                        {
                           cStr tileName;
                           if (pTile->GetName(&tileName) == S_OK)
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
}

////////////////////////////////////////

LRESULT cToolPaletteBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
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

////////////////////////////////////////

LRESULT cToolPaletteBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   UseGlobal(EditorTileManager);
   pEditorTileManager->Disconnect(this);

   m_toolPalette.Clear();

   return 0;
}

////////////////////////////////////////

LRESULT cToolPaletteBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   if (m_toolPalette.IsWindow())
   {
      CRect rect;
      GetClientRect(rect);
      m_toolPalette.MoveWindow(rect);
   }
   return 0;
}

////////////////////////////////////////

LRESULT cToolPaletteBar::OnToolPaletteNotify(int idCtrl, LPNMHDR pnmh, BOOL & bHandled)
{
   Assert(pnmh != NULL);

   sToolPaletteItem tpi;
   if (m_toolPalette.GetTool(((sNMToolPaletteItemClick *)pnmh)->hTool, &tpi))
   {
      switch (pnmh->code)
      {
         case kTPN_ItemCheck:
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
            break;
         }

         case kTPN_ItemUncheck:
         {
            DebugMsg1("Tool \"%s\" un-checked\n", tpi.szName);
            break;
         }

         case kTPN_ItemClick:
         {
            DebugMsg1("Tool \"%s\" clicked\n", tpi.szName);
            break;
         }

         case kTPN_ItemDestroy:
         {
            DebugMsg1("Tool \"%s\" destroyed\n", tpi.szName);
            if (tpi.pUserData != NULL)
            {
               IUnknown * pUnk = reinterpret_cast<IUnknown *>(tpi.pUserData);
               pUnk->Release();
            }
            break;
         }
      }
   }

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
