/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "OutputBar.h"

#include "resource.h"       // main symbols

#include "dbgalloc.h" // must be last header

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

AUTO_REGISTER_DOCKINGWINDOW_SIZED(IDS_OUTPUT_BAR_TITLE, cOutputBar::Factory, kDWP_Bottom, 150, 0);

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
   m_logWnd.AddString(severity, pszMsg, msgLen);

   if (m_nextLogCallback != NULL)
   {
      (*m_nextLogCallback)(severity, pszMsg, msgLen);
   }
}

////////////////////////////////////////

LRESULT cOutputBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   if (!m_logWnd.Create(m_hWnd, CWindow::rcDefault))
   {
      ErrorMsg("Error creating child window\n");
      return -1;
   }

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

////////////////////////////////////////

LRESULT cOutputBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
   m_logWnd.MoveWindow(CRect(CPoint(0,0), CSize(lParam)));
   return 0;
}

/////////////////////////////////////////////////////////////////////////////
