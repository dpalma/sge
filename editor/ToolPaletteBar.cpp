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

static const uint kMaxButtons = 100; // arbitrary - used only for ON_CONTROL_RANGE
static const uint kButtonSize = 32;
static const uint kButtonFirstId = 1000;
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
