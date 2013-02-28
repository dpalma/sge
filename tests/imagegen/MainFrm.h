/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_MAINFRM_H)
#define INCLUDED_MAINFRM_H

#include "igDoc.h"
#include "igView.h"

#include "tech/filespec.h"

#include "resource.h"       // main symbols

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CMainFrame
//

class CMainFrame : public WTL::CFrameWindowImpl<CMainFrame>
                 , public WTL::CUpdateUI<CMainFrame>
                 , public WTL::CMessageFilter
                 , public WTL::CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CMainFrame();
	~CMainFrame();

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
      return WTL::CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle();

	void UpdateTitleBar(LPCTSTR lpstrTitle);

	BEGIN_MSG_MAP_EX(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER_EX(ID_FILE_NEW, OnFileNew)
		COMMAND_RANGE_HANDLER_EX(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)
		COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER_EX(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER_EX(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER_EX(ID_IMAGE_ATTRIBUTES, OnImageAttributes)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_CIRCLE, OnImageCircle)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_RECTANGLE, OnImageRectangle)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_ROUNDRECT, OnImageRoundRect)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_AQUABUTTON, OnImageAquaButton)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_STATIC, OnImageStatic)
      COMMAND_ID_HANDLER_EX(ID_IMAGE_GAMMA, OnImageGamma)
		COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
      CHAIN_MSG_MAP(WTL::CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(WTL::CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_FILE_MRU_FILE1, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_IMAGE_CIRCLE, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_IMAGE_RECTANGLE, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_IMAGE_ROUNDRECT, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_IMAGE_AQUABUTTON, UPDUI_MENUPOPUP)
      UPDATE_ELEMENT(ID_IMAGE_STATIC, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
   void OnFileExit(UINT uNotifyCode, int nID, CWindow wnd);
   void OnFileNew(UINT uNotifyCode, int nID, CWindow wnd);
   void OnFileOpen(UINT uNotifyCode, int nID, CWindow wnd);
   void OnFileRecent(UINT uNotifyCode, int nID, CWindow wnd);
	void OnViewToolBar(UINT uNotifyCode, int nID, CWindow wnd);
	void OnViewStatusBar(UINT uNotifyCode, int nID, CWindow wnd);
   void OnImageAttributes(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageCircle(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageRectangle(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageRoundRect(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageAquaButton(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageStatic(UINT uNotifyCode, int wID, CWindow wnd);
   void OnImageGamma(UINT uNotifyCode, int wID, CWindow wnd);
	void OnAppAbout(UINT uNotifyCode, int nID, CWindow wnd);

private:
   enum
   {
      FILE_MENU_POSITION = 0,
      RECENT_MENU_POSITION = 5,
   };

   cImageGenDoc m_doc;
   cImageGenView m_view;
   cFileSpec m_file;
   WTL::CCommandBarCtrl m_cmdBar;
	WTL::CRecentDocumentList m_mru;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_MAINFRM_H)
