/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAINFRM_H)
#define INCLUDED_MAINFRM_H

#include "editorView.h"

#include <DockingFrame.h>
#include <sstate.h>

#include <vector>

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cDockingWindow;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainFrame
//

class cMainFrame : public dockwins::CDockingFrameImpl<cMainFrame>,
                   public CUpdateUI<cMainFrame>,
                   public CIdleHandler,
                   public CMessageFilter
{
   typedef dockwins::CDockingFrameImpl<cMainFrame> tFrameBase;
   typedef CUpdateUI<cMainFrame> tUpdateUIBase;

   enum
   {
      IDW_DOCKINGWINDOW_FIRST = 0xE800,
      IDW_DOCKINGWINDOW_LAST = 0xE8FF,
      WM_POST_CREATE = WMDF_LAST + 1
   };

public:
   cMainFrame();
   ~cMainFrame();

   DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);

   BEGIN_MSG_MAP_EX(cMainFrame)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_POST_CREATE, OnPostCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
      COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
      COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
      COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
      COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
      COMMAND_RANGE_HANDLER(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)
      COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
      COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
      COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
      COMMAND_ID_HANDLER(ID_TOOLS_UNITTESTRUNNER, OnToolsUnitTestRunner)
      COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
	   COMMAND_RANGE_HANDLER(ID_VIEW_DOCKING_WINDOW1, ID_VIEW_DOCKING_WINDOW16, OnViewControlBar)
      CHAIN_MSG_MAP(tFrameBase)
      CHAIN_MSG_MAP(tUpdateUIBase)
   END_MSG_MAP()

   BEGIN_UPDATE_UI_MAP(cMainFrame)
      //UPDATE_ELEMENT(ID_xxx, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
      UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
	   UPDATE_ELEMENT(ID_VIEW_DOCKING_WINDOW1, UPDUI_MENUPOPUP)
   END_UPDATE_UI_MAP()

   void CreateDockingWindows();

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnPostCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
   LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

   LRESULT OnFileNew(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileOpen(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileSave(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileSaveAs(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileRecent(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnFileExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnViewToolBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnViewStatusBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnToolsUnitTestRunner(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnAppAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);
   LRESULT OnViewControlBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled);

	virtual BOOL PreTranslateMessage(MSG * pMsg);

   virtual BOOL OnIdle();

private:
   CCommandBarCtrl m_cmdBar;
   CRecentDocumentList m_recentDocuments;

   typedef std::vector<cDockingWindow *> tDockingWindows;
   tDockingWindows m_dockingWindows;
   sstate::CWindowStateMgr	m_dockingWindowStateMgr;
   CString m_dockingWindowViewMenuText;

   CComObject<cEditorView> * m_pView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAINFRM_H)
