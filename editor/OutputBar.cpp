/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "outputbar.h"
#include "editorCtrlBars.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

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

////////////////////////////////////////

AUTO_REGISTER_DOCKINGWINDOW(IDS_OUTPUT_BAR_TITLE, cOutputBar::Factory, kDWP_Bottom);

////////////////////////////////////////

tResult cOutputBar::Factory(cDockingWindow * * ppDockingWindow)
{
   if (ppDockingWindow == NULL)
   {
      return E_POINTER;
   }
   cOutputBar * pOutputBar = new cOutputBar;
   if (pOutputBar == NULL)
   {
      return E_OUTOFMEMORY;
   }
   *ppDockingWindow = static_cast<cDockingWindow *>(pOutputBar);
   return S_OK;
}

////////////////////////////////////////

cOutputBar::cOutputBar()
 : m_nextLogCallback(NULL)
{
}

////////////////////////////////////////

cOutputBar::~cOutputBar()
{
}

////////////////////////////////////////

void cOutputBar::HandleLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen)
{
   m_logWnd.AddString(pszMsg, msgLen);

   if (m_nextLogCallback != NULL)
   {
      (*m_nextLogCallback)(severity, pszMsg, msgLen);
   }
}

////////////////////////////////////////

LRESULT cOutputBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   if (m_logWnd.Create(m_hWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE, 0, kChildId) == NULL)
   {
      DebugMsg("Error creating child window\n");
      return -1;
   }

   m_logWnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

#if 0
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
#endif

   g_pOutputBar = this;
   m_nextLogCallback = techlog.SetCallback(OutputBarLogCallback);

   return 0;
}

////////////////////////////////////////

LRESULT cOutputBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   techlog.SetCallback(m_nextLogCallback);
   g_pOutputBar = NULL;
   return NULL;
}
