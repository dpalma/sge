/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_OUTPUTBAR_H
#define INCLUDED_OUTPUTBAR_H

#include "historywnd.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cOutputBar
//

class cOutputBar : public CSizingControlBarG
{
   DECLARE_DYNCREATE(cOutputBar)

// Construction
public:
   cOutputBar();

// Attributes
public:

// Operations
public:
   void HandleLogCallback(eLogSeverity severity, const tChar * pszMsg, size_t msgLen);

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(cOutputBar)
   //}}AFX_VIRTUAL

// Implementation
public:
   virtual ~cOutputBar();

protected:
   cHistoryWnd m_wndChild;

   // Generated message map functions
protected:
   //{{AFX_MSG(cOutputBar)
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()

private:
   tLogCallbackFn m_nextLogCallback;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !INCLUDED_OUTPUTBAR_H
