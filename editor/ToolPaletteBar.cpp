/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPaletteBar.h"

#include "editorCtrlBars.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//const uint kChildId = 256;

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cToolPaletteBar message handlers

int cToolPaletteBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
      return -1;

   SetSCBStyle(GetSCBStyle() | SCBS_SIZECHILD);

//   if (!m_wndChild.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, kChildId))
//   {
//      return -1;
//   }
//
//   m_wndChild.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

   CFont font;
   if (!font.CreateStockObject(DEFAULT_GUI_FONT))
   {
      if (!font.CreatePointFont(80, "MS Sans Serif"))
      {
         DebugMsg("Unable to create font\n");
         return -1;
      }
   }

//   m_wndChild.SetFont(&font);

   return 0;
}

void cToolPaletteBar::OnDestroy() 
{
   CSizingControlBarG::OnDestroy();
}
