/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MainFrm.h"

#include "editorView.h"
#include "editorCtrlBars.h"
#include "aboutdlg.h"

#include <DockMisc.h>

#include "dbgalloc.h" // must be last header

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

#if 0
void CMainFrame::OnUpdateViewControlBarMenu(CCmdUI* pCmdUI) 
{
   if (m_ctrlBarViewMenuText.IsEmpty() && pCmdUI->m_pMenu != NULL)
   {
      pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, m_ctrlBarViewMenuText, MF_BYCOMMAND);
   }

   if (m_ctrlBars.empty())
   {
      if (!m_ctrlBarViewMenuText.IsEmpty())
      {
         pCmdUI->SetText(m_ctrlBarViewMenuText);
      }
      pCmdUI->Enable(FALSE);
      return;
   }

   if (pCmdUI->m_pMenu == NULL)
   {
      return;
   }

   for (uint i = 0; i < m_ctrlBars.size(); i++)
   {
      pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + i, MF_BYCOMMAND);
   }

   CString strName;
   CString strTemp;
   for (i = 0; i < m_ctrlBars.size(); i++)
   {
      ASSERT_VALID(m_ctrlBars[i]);

      CString title, temp;
      m_ctrlBars[i]->GetWindowText(title);

      // double up any '&' characters so they are not underlined
      LPCTSTR lpszSrc = title;
      LPTSTR lpszDest = temp.GetBuffer(title.GetLength()*2);
      while (*lpszSrc != 0)
      {
         if (*lpszSrc == '&')
         {
            *lpszDest++ = '&';
         }
         if (_istlead(*lpszSrc))
         {
            *lpszDest++ = *lpszSrc++;
         }
         *lpszDest++ = *lpszSrc++;
      }
      *lpszDest = 0;
      temp.ReleaseBuffer();

      uint menuFlags = MF_STRING | MF_BYPOSITION;

      if (m_ctrlBars[i]->IsWindowVisible())
      {
         menuFlags |= MF_CHECKED;
      }

      pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex++, menuFlags, pCmdUI->m_nID++, temp);
   }

   // update end menu count
   pCmdUI->m_nIndex--; // point to last menu added
   pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();

   pCmdUI->m_bEnableChanged = TRUE; // all the added items are enabled
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMainFrame
//

////////////////////////////////////////

void cMainFrame::CreateDockingWindows()
{
   static const uint ctrlBarPlacementMap[] =
   {
      dockwins::CDockingSide::sTop, // kCBP_Top
      dockwins::CDockingSide::sLeft, // kCBP_Left
      dockwins::CDockingSide::sRight, //kCBP_Right
      dockwins::CDockingSide::sBottom, //kCBP_Bottom
      dockwins::CDockingSide::sInvalid, //kCBP_Float
   };

   uint titleStringId;
   tDockingWindowFactoryFn factoryFn;
   eControlBarPlacement placement;

   uint ctrlBarId = IDW_DOCKINGWINDOW_FIRST + 32;

   const CRect rect(0,0,0,0);

   static const DWORD ctrlBarStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;

//   const CString ctrlBarWndClass = AfxRegisterWndClass(CS_DBLCLKS,
//      LoadCursor(NULL, IDC_ARROW), GetSysColorBrush(COLOR_BTNFACE), 0);

   std::vector<uint> dockBars;
   HANDLE hIter;
   IterCtrlBarsBegin(&hIter);
   while (IterNextCtrlBar(&hIter, &titleStringId, &factoryFn, &placement))
   {
      CString title;
      if (!title.LoadString(titleStringId))
      {
         title.Format("DockingWindow%d", titleStringId);
      }

      if (factoryFn != NULL && !IsBadCodePtr(reinterpret_cast<FARPROC>(factoryFn)))
      {
         cDockingWindow * pCtrlBar = NULL;
         if (((*factoryFn)(&pCtrlBar) == S_OK) && (pCtrlBar != NULL))
         {
            // TODO: use ctrlBarId
		      if (pCtrlBar->Create(m_hWnd, rcDefault, title, ctrlBarStyle))
            {
		         DockWindow(*pCtrlBar, dockwins::CDockingSide(ctrlBarPlacementMap[placement]),
						         0/*nBar*/,float(0.0)/*fPctPos*/,100/*nWidth*/,100/* nHeight*/);
               ctrlBarId++;
               m_dockingWindows.push_back(pCtrlBar);
            }
            else
            {
               WarnMsg("Error creating docking window\n");
               delete pCtrlBar;
            }

//            if (pCtrlBar->Create(ctrlBarWndClass, title, ctrlBarStyle, rect, this, ctrlBarId))
//            {
//               pCtrlBar->EnableDocking(CBRS_ALIGN_ANY);
//#if _MFC_VER < 0x0700
//               pCtrlBar->SetBarStyle(pCtrlBar->GetBarStyle() |
//                  CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
//#endif
//
//               ctrlBarId++;
//               m_ctrlBars.push_back(pCtrlBar);
//               dockBars.push_back(ctrlBarPlacementMap[placement]);
//            }
//            else
//            {
//               DebugMsg1("Error creating control bar of type %s\n", pRuntimeClass->m_lpszClassName);
//               delete pCtrlBar;
//            }
         }
      }
   }
   IterCtrlBarsEnd(hIter);

//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);
//
//   Assert(m_ctrlBars.size() == dockBars.size());
//
//   for (uint i = 0; i < m_ctrlBars.size(); i++)
//   {
//	   DockControlBar(m_ctrlBars[i], dockBars[i]);
//   }
}

////////////////////////////////////////

LRESULT cMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   HWND hWndCmdBar = m_cmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
   if (hWndCmdBar == NULL)
   {
      ErrorMsg("Error creating command bar\n");
      return -1;
   }

   m_cmdBar.AttachMenu(GetMenu());
   m_cmdBar.LoadImages(IDR_MAINFRAME);
   SetMenu(NULL);

   if (!CreateSimpleStatusBar())
   {
      ErrorMsg("Error creating status bar\n");
      return -1;
   }

   HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
   if (hWndToolBar == NULL)
   {
      ErrorMsg("Error creating toolbar\n");
      return -1;
   }

   UIAddToolBar(hWndToolBar);

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->AddIdleHandler(this);
   pMessageLoop->AddMessageFilter(this);

   InitializeDockingFrame();

   CreateDockingWindows();

   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   tDockingWindows::iterator iter = m_dockingWindows.begin();
   tDockingWindows::iterator end = m_dockingWindows.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }

   m_dockingWindows.clear();

   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileNew(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileOpen(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileSave(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileSaveAs(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileRecent(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnFileExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   PostQuitMessage(0);
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnViewToolBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnViewStatusBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   // TODO
   return 0;
}

////////////////////////////////////////

extern void RunUnitTests(); // editorApp.cpp
LRESULT cMainFrame::OnToolsUnitTestRunner(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   RunUnitTests();
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnAppAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
	cAboutDlg().DoModal();
   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnViewControlBar(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   Assert(id >= ID_VIEW_CONTROL_BAR1);
   Assert(id < (ID_VIEW_CONTROL_BAR1 + m_dockingWindows.size()));
   int index = id - ID_VIEW_CONTROL_BAR1;
   Assert(m_dockingWindows[index] != NULL);
   // TODO
//   CControlBar * pCtrlBar = m_ctrlBars[index];
//   ShowControlBar(pCtrlBar, !pCtrlBar->IsWindowVisible(), FALSE);
   return 0;
}

////////////////////////////////////////

BOOL cMainFrame::PreTranslateMessage(MSG * pMsg)
{
   return FALSE;
}

////////////////////////////////////////

BOOL cMainFrame::OnIdle()
{
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
