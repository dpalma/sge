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

cOutputBar * g_pOutputBar = NULL;

void OutputBarLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen)
{
   if (g_pOutputBar != NULL)
   {
      g_pOutputBar->HandleLogCallback(severity, pszMsg, msgLen);
   }
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOutputBar
//

cOutputBar::cOutputBar()
 : m_nextLogCallback(NULL)
{
}

cOutputBar::~cOutputBar()
{
}

static const COLORREF g_colors[] =
{
   RGB(255,0,0),     // kError
   RGB(0,255,255),   // kWarning
   RGB(0,0,0),       // kInfo
   RGB(0,0,255),     // kDebug
};

void cOutputBar::HandleLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen)
{
   m_wndChild.AddLine(pszMsg, g_colors[severity]);

   if (m_nextLogCallback != NULL)
   {
      (*m_nextLogCallback)(severity, pszMsg, msgLen);
   }
}

BEGIN_MESSAGE_MAP(cOutputBar, CSizingControlBarG)
   //{{AFX_MSG_MAP(cOutputBar)
   ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cOutputBar message handlers

int cOutputBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CSizingControlBarG::OnCreate(lpCreateStruct) == -1)
      return -1;

   SetSCBStyle(GetSCBStyle() | SCBS_SIZECHILD);

   if (!m_wndChild.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, kChildId))
   {
      return -1;
   }

   m_wndChild.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

   CFont font;
   if (!font.CreateStockObject(DEFAULT_GUI_FONT))
   {
      if (!font.CreatePointFont(80, "MS Sans Serif"))
      {
         DebugMsg("Unable to create font\n");
         return -1;
      }
   }

   m_wndChild.SetFont(&font);

   g_pOutputBar = this;
   m_nextLogCallback = techlog.SetCallback(OutputBarLogCallback);

   return 0;
}

void cOutputBar::OnDestroy() 
{
   CSizingControlBarG::OnDestroy();

   techlog.SetCallback(m_nextLogCallback);
   g_pOutputBar = NULL;
}
