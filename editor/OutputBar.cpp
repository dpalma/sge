/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "outputbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const uint kChildId = 256;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOutputBar
//

cOutputBar::cOutputBar()
{
}

cOutputBar::~cOutputBar()
{
}

BEGIN_MESSAGE_MAP(cOutputBar, CSizingControlBarG)
   //{{AFX_MSG_MAP(cOutputBar)
   ON_WM_CREATE()
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cOutputBar message handlers

int cOutputBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
      return -1;

   SetSCBStyle(GetSCBStyle() | SCBS_SIZECHILD);

   if (!m_wndChild.Create(WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL,
      CRect(0,0,0,0), this, kChildId))
      return -1;

   m_wndChild.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

   // older versions of Windows* (NT 3.51 for instance)
   // fail with DEFAULT_GUI_FONT
   if (!m_font.CreateStockObject(DEFAULT_GUI_FONT))
      if (!m_font.CreatePointFont(80, "MS Sans Serif"))
         return -1;

   m_wndChild.SetFont(&m_font);

   return 0;
}
