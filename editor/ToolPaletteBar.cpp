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

                        m_buttons.push_back(pButton);
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
   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      (*iter)->DestroyWindow();
      delete *iter;
   }
   m_buttons.clear();
}

void cToolPaletteBar::RepositionButtons(BOOL bRepaint)
{
   CRect rect;
   GetClientRect(rect);

   CRect buttonRect;
   buttonRect.left = rect.left + buttonMargins.left;
   buttonRect.top = rect.top + buttonMargins.top;
   buttonRect.right = buttonRect.left + kButtonSize;
   buttonRect.bottom = buttonRect.top + kButtonSize;

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->GetSafeHwnd()))
      {
         (*iter)->MoveWindow(buttonRect, bRepaint);

         buttonRect.OffsetRect(buttonRect.Width(), 0);

         if (buttonRect.left > (rect.right - buttonMargins.right)
            || buttonRect.right > (rect.right - buttonMargins.right))
         {
            buttonRect.left = rect.left + buttonMargins.left;
            buttonRect.top += kButtonSize;;
            buttonRect.right = buttonRect.left + kButtonSize;
            buttonRect.bottom = buttonRect.top + kButtonSize;
         }
      }
   }
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
   Assert(buttonId >= kButtonFirstId && buttonId < (kButtonFirstId + m_buttons.size()));

   DebugMsg1("Button %d clicked\n", buttonId);

   tButtons::iterator iter;
   for (iter = m_buttons.begin(); iter != m_buttons.end(); iter++)
   {
      if (((*iter) != NULL) && IsWindow((*iter)->GetSafeHwnd()) && (*iter)->GetDlgCtrlID() != buttonId)
      {
         (*iter)->SendMessage(BM_SETSTATE, FALSE);
      }
   }

   ASSERT_VALID(m_buttons[buttonId - kButtonFirstId]);
   m_buttons[buttonId - kButtonFirstId]->SendMessage(BM_SETSTATE, TRUE);
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
