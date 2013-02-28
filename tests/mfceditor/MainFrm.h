/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAINFRM_H)
#define INCLUDED_MAINFRM_H

#include <vector>

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CMainFrame
//

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE_EX(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnDestroy();
	afx_msg void OnUpdateViewControlBarMenu(CCmdUI* pCmdUI);
	//}}AFX_MSG
   afx_msg BOOL OnViewControlBar(UINT nID);
	DECLARE_MESSAGE_MAP()

private:
   std::vector<CControlBar *> m_ctrlBars;
   CString m_ctrlBarViewMenuText;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAINFRM_H)
