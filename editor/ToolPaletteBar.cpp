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
 : m_hTerrainTileGroup(NULL)
{
}

////////////////////////////////////////

cToolPaletteBar::~cToolPaletteBar()
{
}

////////////////////////////////////////

void cToolPaletteBar::OnDefaultTileSetChange(IEditorTileSet * pTileSet)
{
   if (m_toolPalette.IsWindow() && (m_hTerrainTileGroup != NULL))
   {
      Verify(m_toolPalette.RemoveGroup(m_hTerrainTileGroup));
      m_hTerrainTileGroup = NULL;
   }

   if (pTileSet != NULL)
   {
      uint nTiles = 0;
      if (pTileSet->GetTileCount(&nTiles) == S_OK)
      {
         if (m_toolPalette.IsWindow())
         {
            cStr name;
            HIMAGELIST hImageList = NULL;
            if (pTileSet->GetName(&name) == S_OK
               && pTileSet->GetImageList(16, &hImageList) == S_OK)
            {
               // cToolPalette::AddGroup() expects to take ownership of image
               // list, by the tileset stores it internally; hence, the
               // ImageList_Duplicate() call.
               m_hTerrainTileGroup = m_toolPalette.AddGroup(name.c_str(),
                  ImageList_Duplicate(hImageList));
            }
         }

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
                  if (m_toolPalette.IsWindow() && (m_hTerrainTileGroup != NULL))
                  {
                     m_toolPalette.AddTool(m_hTerrainTileGroup, tileName.c_str(), i);
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
   if (!m_tooltip.Create(m_hWnd))
   {
      ErrorMsg1("Unable to create tooltip control (error %d)\n", GetLastError());
      return -1;
   }

   if (!m_toolPalette.Create(m_hWnd, CWindow::rcDefault, "", 0, 0, kToolPaletteId))
   {
      ErrorMsg1("Unable to create tool palette control (error %d)\n", GetLastError());
      return -1;
   }

   ////////////////////////////////////////
   ////////////////////////////////////////
   // TESTING
   ////////////////////////////////////////
#if 1
   for (int i = 0; i < 3; i++)
   {
      tChar szTemp[200];
      if (i == 0)
      {
         ZeroMemory(szTemp, sizeof(szTemp));
      }
      else
      {
         wsprintf(szTemp, "Group %c", 'A' + i);
      }
      HTOOLGROUP hToolGroup = m_toolPalette.AddGroup(szTemp, NULL);
      if (hToolGroup != NULL)
      {
         int nTools = 3 + (rand() & 3);
         DebugMsg2("%s has %d tools\n", szTemp, nTools);
         for (int j = 0; j < nTools; j++)
         {
            wsprintf(szTemp, "Tool %d", j);
            HTOOLITEM hTool = m_toolPalette.AddTool(hToolGroup, szTemp, -1);
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

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->AddMessageFilter(this);

   return 0;
}

////////////////////////////////////////

LRESULT cToolPaletteBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->RemoveMessageFilter(this);

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

   std::string temp;

   switch (pnmh->code)
   {
      case kTPN_ItemClick:
      {
         if (m_toolPalette.GetToolText(((sNMToolPaletteItemClick *)pnmh)->hTool, &temp))
         {
            DebugMsg1("Tool \"%s\" clicked\n", temp.c_str());
         }
         break;
      }

      case kTPN_ItemDestroy:
      {
         // TODO: release/free whatever is in user data
         break;
      }
   }

   return 0;
}

////////////////////////////////////////

BOOL cToolPaletteBar::PreTranslateMessage(MSG* pMsg) 
{
   if (m_tooltip.IsWindow())
   {
      if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
      {
         m_tooltip.RelayEvent(pMsg);
      }
   }

   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
