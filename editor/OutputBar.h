/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_OUTPUTBAR_H
#define INCLUDED_OUTPUTBAR_H

#include "LogWnd.h"
#include "editorDockingWindows.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOutputBar
//

class cOutputBar : public cDockingWindow
{
public:
   static tResult Factory(cDockingWindow * * ppDockingWindow);

   cOutputBar();
   ~cOutputBar();

   void HandleLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen);

   BEGIN_MSG_MAP(cOutputBar)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      CHAIN_MSG_MAP(cDockingWindow)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

private:
   cLogWnd m_logWnd;
   tLogCallbackFn m_nextLogCallback;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_OUTPUTBAR_H
