/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MainFrm.h"

#include "ImageAttributesDlg.h"
#include "ImageGammaDlg.h"

#include <atldlgs.h>

LPCTSTR g_szMRURegKey = _T("Software\\ImageGen");
LPCTSTR g_szApp = _T("ImageGen");


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: CMainFrame
//

CMainFrame::CMainFrame()
 : m_view(&m_doc)
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::OnIdle()
{
	UIEnable(ID_FILE_MRU_FILE1, m_mru.m_arrDocs.GetSize() > 0);
   UISetCheck(ID_IMAGE_CIRCLE, m_doc.GetShape() == cImageGenDoc::Circle);
   UISetCheck(ID_IMAGE_RECTANGLE, m_doc.GetShape() == cImageGenDoc::Rectangle);
   UISetCheck(ID_IMAGE_ROUNDRECT, m_doc.GetShape() == cImageGenDoc::RoundRect);
   UISetCheck(ID_IMAGE_AQUABUTTON, m_doc.GetShape() == cImageGenDoc::AquaButton);
   UISetCheck(ID_IMAGE_STATIC, m_doc.GetShape() == cImageGenDoc::Static);
	UIUpdateToolBar();
	return FALSE;
}

void CMainFrame::UpdateTitleBar(LPCTSTR lpstrTitle)
{
   WTL::CString strDefault;
	strDefault.LoadString(IDR_MAINFRAME);
	WTL::CString strTitle = strDefault;
	if (lpstrTitle != NULL)
	{
		strTitle = lpstrTitle;
		strTitle += _T(" - ");
		strTitle += strDefault;
	}
	SetWindowText(strTitle);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

int CMainFrame::OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
{
	// create command bar window
	HWND hWndCmdBar = m_cmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_cmdBar.AttachMenu(GetMenu());
	m_cmdBar.LoadImages(IDR_MAINFRAME);
	SetMenu(NULL);

	// create toolbar and rebar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	// create status bar
	CreateSimpleStatusBar();

	// create view window
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	// set up MRU stuff
   WTL::CMenuHandle menu = m_cmdBar.GetMenu();
	WTL::CMenuHandle menuFile = menu.GetSubMenu(FILE_MENU_POSITION);
	WTL::CMenuHandle menuMru = menuFile.GetSubMenu(RECENT_MENU_POSITION);
	m_mru.SetMenuHandle(menuMru);
	m_mru.SetMaxEntries(12);

	m_mru.ReadFromRegistry(g_szMRURegKey);

	// set up update UI
	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

   WTL::CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

   m_doc.NewDocument();
   UpdateTitleBar(NULL);
   m_view.Update();

	return 0;
}

void CMainFrame::OnFileExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	PostMessage(WM_CLOSE);
}

void CMainFrame::OnFileOpen(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
   WTL::CString imageFileFilter;
	if (imageFileFilter.LoadString(IDS_IMAGE_FILE_FILTER))
   {
      int len = imageFileFilter.GetLength();
      tChar * psz = imageFileFilter.GetBuffer(1);
      for (int i = 0; i < len; ++i)
      {
         if (_T('|') == psz[i])
            psz[i] = _T('\0');
      }
	   imageFileFilter.ReleaseBuffer();
   }

   WTL::CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, imageFileFilter, m_hWnd);
	if (dlg.DoModal() == IDOK)
	{
      if (m_doc.OpenDocument(dlg.m_szFileName))
      {
			m_view.Update();
			m_mru.AddToList(dlg.m_ofn.lpstrFile);
			UpdateTitleBar(dlg.m_szFileTitle);
      }
	}
}

void CMainFrame::OnFileNew(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
   m_doc.NewDocument();
   UpdateTitleBar(NULL);
   m_view.Update();
}

void CMainFrame::OnFileRecent(UINT /*uNotifyCode*/, int nID, CWindow /*wnd*/)
{
	TCHAR szFile[MAX_PATH];
	if (m_mru.GetFromList(nID, szFile))
	{
      if (m_doc.OpenDocument(szFile))
      {
         m_view.Update();
			m_mru.MoveToTop(nID);
			UpdateTitleBar(szFile);
      }
      else
      {
			m_mru.RemoveFromList(nID);
      }
	}
	else
	{
		::MessageBeep(MB_ICONERROR);
	}
}

void CMainFrame::OnViewToolBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	static BOOL bNew = TRUE;	// initially visible
	bNew = !bNew;
	uint uBandID = ATL_IDW_BAND_FIRST + 1;	// toolbar is second added band
   WTL::CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(uBandID);
	rebar.ShowBand(nBandIndex, bNew);
	UISetCheck(ID_VIEW_TOOLBAR, bNew);
	UpdateLayout();
}

void CMainFrame::OnViewStatusBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	BOOL bNew = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bNew);
	UpdateLayout();
}

void CMainFrame::OnImageAttributes(UINT uNotifyCode, int wID, CWindow wnd)
{
   if (m_doc.AccessImage() == NULL)
   {
      return;
   }

   cImageAttributesDlg dlg;
   dlg.m_pixelFormat = m_doc.AccessImage()->GetPixelFormat();
   dlg.m_width = m_doc.AccessImage()->GetWidth();
   dlg.m_height = m_doc.AccessImage()->GetHeight();
   if (dlg.DoModal() == IDOK)
   {
      cAutoIPtr<IImage> pNewImage;
      if (ImageCreate(dlg.m_width, dlg.m_height, (ePixelFormat)dlg.m_pixelFormat, NULL, &pNewImage) == S_OK)
      {
         m_doc.SetImage(pNewImage);
         m_view.Update();
      }
   }
}

void CMainFrame::OnImageCircle(UINT uNotifyCode, int wID, CWindow wnd)
{
   m_doc.SetShape(cImageGenDoc::Circle);
   m_view.Update();
}

void CMainFrame::OnImageRectangle(UINT uNotifyCode, int wID, CWindow wnd)
{
   m_doc.SetShape(cImageGenDoc::Rectangle);
   m_view.Update();
}

void CMainFrame::OnImageRoundRect(UINT uNotifyCode, int wID, CWindow wnd)
{
   m_doc.SetShape(cImageGenDoc::RoundRect);
   m_view.Update();
}

void CMainFrame::OnImageAquaButton(UINT uNotifyCode, int wID, CWindow wnd)
{
   m_doc.SetShape(cImageGenDoc::AquaButton);
   m_view.Update();
}

void CMainFrame::OnImageStatic(UINT uNotifyCode, int wID, CWindow wnd)
{
   m_doc.SetShape(cImageGenDoc::Static);
   m_view.Update();
}

void CMainFrame::OnImageGamma(UINT uNotifyCode, int wID, CWindow wnd)
{
   cImageGammaDlg dlg;
   dlg.m_bApplyGamma = m_doc.IsGammaEnabled();
   dlg.m_gamma = m_doc.GetGamma();
   if (dlg.DoModal() == IDOK)
   {
      m_doc.EnableGamma(dlg.m_bApplyGamma ? true : false);
      m_doc.SetGamma(dlg.m_gamma);
      m_view.Update();
   }
}

void CMainFrame::OnAppAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
{
	CSimpleDialog<IDD_ABOUTBOX>().DoModal();
}

/////////////////////////////////////////////////////////////////////////////
