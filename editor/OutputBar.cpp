/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "outputbar.h"

#include "editorCtrlBars.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
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

AUTO_REGISTER_CONTROLBAR(IDS_OUTPUT_BAR_TITLE, RUNTIME_CLASS(cOutputBar), kCBP_Bottom);

IMPLEMENT_DYNCREATE_EX(cOutputBar, CSizingControlBarG);

cOutputBar::cOutputBar()
 : m_nextLogCallback(NULL)
{
}

cOutputBar::~cOutputBar()
{
}

void cOutputBar::HandleLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen)
{
   static const COLORREF colors[] =
   {
      RGB(255,0,0),                    // kError
      GetSysColor(COLOR_WINDOWTEXT),   // kWarning
      GetSysColor(COLOR_WINDOWTEXT),   // kInfo
      RGB(0,0,255),                    // kDebug
   };

   m_wndChild.AddText(pszMsg, msgLen, colors[severity]);

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

   CFont font;
   if (!font.CreateStockObject(DEFAULT_GUI_FONT))
   {
      if (!font.CreatePointFont(80, "MS Sans Serif"))
      {
         DebugMsg("Unable to create font\n");
         return -1;
      }
   }

   if (!m_wndChild.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, kChildId))
   {
      return -1;
   }

   m_wndChild.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

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
