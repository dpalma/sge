/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "globalobj.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

static const uint kButtonSize = 32;
static const CRect buttonMargins(5,5,5,5);

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
   Assert(pButton != NULL);
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

int cButtonPanel::Reposition(LPCRECT pRect, BOOL bRepaint)
{
   CRect buttonRect;
   buttonRect.left = pRect->left + m_margins.left;
   buttonRect.top = pRect->top + m_margins.top;
   buttonRect.right = buttonRect.left + kButtonSize;
   buttonRect.bottom = buttonRect.top + kButtonSize;

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->m_hWnd))
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

   return buttonRect.bottom;
}

////////////////////////////////////////

void cButtonPanel::HandleClick(uint buttonId)
{
   Assert(!!m_pTool);

   UseGlobal(EditorApp);
   pEditorApp->SetActiveTool(m_pTool);

   m_pTool->SetTile(buttonId - m_buttons[0]->GetDlgCtrlID());

   CButton * pClickedButton = NULL;

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->m_hWnd) && (*iter)->GetDlgCtrlID() != buttonId)
      {
         (*iter)->SendMessage(BM_SETSTATE, FALSE);
      }

      if ((*iter)->GetDlgCtrlID() == buttonId)
      {
         pClickedButton = *iter;
      }
   }

   Assert(pClickedButton != NULL);
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
         m_toolPalette.Clear();
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
                     for (uint j = 0; j < nTiles; j++)
                     {
                        cAutoIPtr<IEditorTile> pTile;
                        if (pTileSet->GetTile(j, &pTile) == S_OK)
                        {
                           cStr tileName;
                           if (pTile->GetName(&tileName) == S_OK)
                           {
                              m_toolPalette.AddTool(hGroup, tileName.c_str(), j);
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
         HTOOLITEM hTool = m_toolPalette.AddTool(hStdGroup, "Select", 0, NULL);
         if (hTool != NULL)
         {
         }
      }
   }

   ////////////////////////////////////////
   ////////////////////////////////////////
   // FOR TESTING
   ////////////////////////////////////////
#if 1
   for (int i = 0; i < 3; i++)
   {
      tChar szTemp[200];
      wsprintf(szTemp, "Group %c", 'A' + i);
      HTOOLGROUP hToolGroup = m_toolPalette.AddGroup(szTemp, NULL);
      if (hToolGroup != NULL)
      {
         int nTools = 3 + (rand() & 3);
         DebugMsg2("%s has %d tools\n", szTemp, nTools);
         for (int j = 0; j < nTools; j++)
         {
            wsprintf(szTemp, "Tool %d", j);
            HTOOLITEM hTool = m_toolPalette.AddTool(hToolGroup, szTemp, -1);
            if (hTool != NULL)
            {
               if (j == 1)
               {
                  Verify(m_toolPalette.EnableTool(hTool, false));
               }
            }
         }
      }
   }
#endif
   ////////////////////////////////////////
   // END TESTING
   ////////////////////////////////////////
   ////////////////////////////////////////

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
            DebugMsg1("Tool \"%s\" checked\n", tpi.szName);
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
            // TODO: release/free whatever is in user data
            DebugMsg1("Tool \"%s\" destroyed\n", tpi.szName);
            break;
         }
      }
   }

   return 0;
}

/////////////////////////////////////////////////////////////////////////////
