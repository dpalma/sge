/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAINFRM_H)
#define INCLUDED_MAINFRM_H

#include "editorView.h"

#include <atlframe.h>
#include <atlmisc.h>

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
	DECLARE_DYNCREATE(CMainFrame)

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
	afx_msg void OnUpdateViewControlBarMenu(CCmdUI* pCmdUI);
	//}}AFX_MSG
   afx_msg BOOL OnViewControlBar(UINT nID);
	DECLARE_MESSAGE_MAP()

private:
   std::vector<CControlBar *> m_ctrlBars;
   CString m_ctrlBarViewMenuText;
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainFrame
//

class cMainFrame : public WTL::CFrameWindowImpl<cMainFrame>,
                   public WTL::CUpdateUI<cMainFrame>,
                   public WTL::CIdleHandler,
                   public WTL::CMessageFilter
{
   typedef WTL::CFrameWindowImpl<cMainFrame> tFrameBase;
   typedef WTL::CUpdateUI<cMainFrame> tUpdateUIBase;

public:
   DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);

   BEGIN_MSG_MAP_EX(cMainFrame)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
      COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
      COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
      COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
      COMMAND_RANGE_HANDLER(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)
      COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
      COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
      COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
      COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
      CHAIN_MSG_MAP(tFrameBase)
      CHAIN_MSG_MAP(tUpdateUIBase)
   END_MSG_MAP()

   BEGIN_UPDATE_UI_MAP(cMainFrame)
      //UPDATE_ELEMENT(ID_xxx, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
      UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
   END_UPDATE_UI_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

   LRESULT OnFileNew(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileOpen(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileSave(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileSaveAs(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileRecent(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnViewToolBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnViewStatusBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnAppAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);

	virtual BOOL PreTranslateMessage(MSG * pMsg);

   virtual BOOL OnIdle();

private:
//   WTL::CCommandBarCtrl m_cmdBar;
   WTL::CRecentDocumentList m_recentDocuments;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAINFRM_H)
