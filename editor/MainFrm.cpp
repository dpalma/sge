/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MainFrm.h"

#include "editorView.h"
#include "editorCtrlBars.h"
#include "aboutdlg.h"

#include <DockMisc.h>
#include <dwstate.h>

#include "dbgalloc.h" // must be last header

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDockingWindowMenu
//

////////////////////////////////////////

cDockingWindowMenu::cDockingWindowMenu(const tDockingWindows & dockingWindows,
                                       uint idFirst, uint idLast)
 : m_dockingWindows(dockingWindows),
   m_idFirst(idFirst),
   m_idLast(idLast)
{
}

////////////////////////////////////////

cDockingWindowMenu::~cDockingWindowMenu()
{
}

////////////////////////////////////////

void cDockingWindowMenu::SetMenu(HMENU hMenu)
{
   m_menu = hMenu;
   m_originalText.Empty();
}

////////////////////////////////////////

static tResult GetMenuItemPosition(HMENU hMenu, uint command, int * pPosition)
{
   if (!IsMenu(hMenu))
   {
      return E_INVALIDARG;
   }

   if (pPosition == NULL)
   {
      return E_POINTER;
   }

   int nItems = GetMenuItemCount(hMenu);

   for (int i = 0; i < nItems; i++)
   {
      CMenuItemInfo mii;
      mii.fMask = MIIM_ID;
      if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
      {
         if (mii.wID == command)
         {
            *pPosition = i;
            return S_OK;
         }
      }
   }

   return E_FAIL;
}

bool cDockingWindowMenu::UpdateMenu()
{
   if (m_menu.IsNull())
   {
      return false;
   }

   if (m_originalText.IsEmpty())
   {
      if (!m_menu.GetMenuString(m_idFirst, m_originalText, MF_BYCOMMAND))
      {
         m_originalText = _T("No Entries");
      }
   }

   int insertPoint;
   if (GetMenuItemPosition(m_menu, m_idFirst, &insertPoint) != S_OK)
   {
      WarnMsg1("No menu item with ID %d\n", m_idFirst);
      return false;
   }

   for (uint i = m_idFirst; i <= m_idLast; i++)
   {
      m_menu.DeleteMenu(i, MF_BYCOMMAND);
   }

   if (m_dockingWindows.empty())
   {
      if (!m_originalText.IsEmpty())
      {
         CMenuItemInfo mii;
         mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
         mii.wID = m_idFirst;
         mii.fState = MFS_DISABLED;
         mii.dwTypeData = const_cast<LPSTR>((LPCTSTR)m_originalText);
         mii.cch = m_originalText.GetLength();
         if (m_menu.InsertMenuItem(insertPoint, TRUE, &mii))
         {
            return true;
         }
      }
   }
   else
   {
      tDockingWindows::const_iterator iter = m_dockingWindows.begin();
      tDockingWindows::const_iterator end = m_dockingWindows.end();
      for (int index = 0; iter != end; iter++, index++)
      {
         uint len = (*iter)->GetWindowTextLength() + 1;
         CString title;
         (*iter)->GetWindowText(title.GetBuffer(len), len);

         // TODO: double up all '&' characters so they don't appear underlined

         CMenuItemInfo mii;
         mii.fMask = MIIM_ID | MIIM_STRING;
         mii.wID = m_idFirst + index;
         mii.dwTypeData = const_cast<LPSTR>((LPCTSTR)title);
         mii.cch = title.GetLength();
         if ((*iter)->IsWindowVisible())
         {
            mii.fMask |= MIIM_STATE;
            mii.fState |= MFS_CHECKED;
         }
         if (!m_menu.InsertMenuItem(insertPoint + index, TRUE, &mii))
         {
            return false;
         }
      }
   }

   return true;
}

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

cMainFrame::cMainFrame()
 : m_dockingWindowMenu(m_dockingWindows, ID_VIEW_DOCKING_WINDOW_FIRST, ID_VIEW_DOCKING_WINDOW_LAST)
{
}

////////////////////////////////////////

cMainFrame::~cMainFrame()
{
}

////////////////////////////////////////

void cMainFrame::CreateDockingWindows()
{
   static const uint placementMap[] =
   {
      dockwins::CDockingSide::sTop, // kCBP_Top
      dockwins::CDockingSide::sLeft, // kCBP_Left
      dockwins::CDockingSide::sRight, //kCBP_Right
      dockwins::CDockingSide::sBottom, //kCBP_Bottom
      dockwins::CDockingSide::sInvalid, //kCBP_Float
   };

   uint titleStringId;
   tDockingWindowFactoryFn factoryFn;
   eDockingWindowPlacement placement;

   CRect defaultDockRect(0, 0, 100, 100);

   std::vector<uint> dockSides;

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
         cDockingWindow * pDockingWindow = NULL;
         if (((*factoryFn)(&pDockingWindow) == S_OK) && (pDockingWindow != NULL))
         {
            if (pDockingWindow->Create(m_hWnd, defaultDockRect, title))
            {
               dockSides.push_back(placementMap[placement]);
               m_dockingWindows.push_back(pDockingWindow);
            }
            else
            {
               WarnMsg("Error creating docking window\n");
               delete pDockingWindow;
            }
         }
      }
   }
   IterCtrlBarsEnd(hIter);

   Assert(m_dockingWindows.size() == dockSides.size());

   uint index = 0;
   tDockingWindows::iterator iter = m_dockingWindows.begin();
   tDockingWindows::iterator end = m_dockingWindows.end();
   for (; iter != end; iter++, index++)
   {
      cDockingWindow * pDockingWindow = *iter;
      DockWindow(*pDockingWindow, dockwins::CDockingSide(dockSides[index]),
         0, 0.0f/*fPctPos*/,100/*nWidth*/,100/* nHeight*/);
   }
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

   // TODO: get rid of hard-coded two
   HMENU hViewMenu = GetSubMenu(m_cmdBar.GetMenu(), 2);
   m_dockingWindowMenu.SetMenu(hViewMenu);

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

   CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
   AddSimpleReBarBand(hWndCmdBar);
   AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

   UIAddToolBar(hWndToolBar);

   m_hWndClient = m_clientWnd.Create(m_hWnd, rcDefault, NULL, 0, 0, ATL_IDW_CLIENT);
   if (m_hWndClient == NULL)
   {
      ErrorMsg("Error creating view window\n");
      return -1;
   }

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->AddIdleHandler(this);
   pMessageLoop->AddMessageFilter(this);

   InitializeDockingFrame();

   CreateDockingWindows();

   PostMessage(WM_POST_CREATE);

   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   sstate::CDockWndMgr dockWndMgr;
   tDockingWindows::iterator iter = m_dockingWindows.begin();
   tDockingWindows::iterator end = m_dockingWindows.end();
   for (; iter != end; iter++)
   {
      dockWndMgr.Add(sstate::CDockingWindowStateAdapter<cDockingWindow>(*(*iter)));
   }

   m_dockingWindowStateMgr.Initialize(_T("SOFTWARE\\SGE"), m_hWnd);
//   m_dockingWindowStateMgr.Add(sstate::CRebarStateAdapter(m_hWndToolBar));
   m_dockingWindowStateMgr.Add(sstate::CToggleWindowAdapter(m_hWndStatusBar));
   m_dockingWindowStateMgr.Add(dockWndMgr);
   m_dockingWindowStateMgr.Restore();

   UpdateLayout();

   return 0;
}

////////////////////////////////////////

LRESULT cMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
   tDockingWindows::iterator iter = m_dockingWindows.begin();
   tDockingWindows::iterator end = m_dockingWindows.end();
   for (; iter != end; iter++)
   {
      (*iter)->DestroyWindow();
      delete *iter;
   }

   m_dockingWindows.clear();

   PostQuitMessage(0);

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
   DestroyWindow();
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

LRESULT cMainFrame::OnViewDockingWindow(WORD notifyCode, WORD id, HWND hWndCtl, BOOL & bHandled)
{
   Assert(id >= ID_VIEW_DOCKING_WINDOW1);
   Assert(id < (ID_VIEW_DOCKING_WINDOW1 + m_dockingWindows.size()));
   int index = id - ID_VIEW_DOCKING_WINDOW1;
   Assert(m_dockingWindows[index] != NULL);
   m_dockingWindows[index]->Toggle();
   return 0;
}

////////////////////////////////////////

BOOL cMainFrame::PreTranslateMessage(MSG * pMsg)
{
   if (tFrameBase::PreTranslateMessage(pMsg))
   {
      return TRUE;
   }

//   if ((m_pView != NULL) && m_pView->PreTranslateMessage(pMsg))
//   {
//      return TRUE;
//   }

   return FALSE;
}

////////////////////////////////////////

BOOL cMainFrame::OnIdle()
{
   UIUpdateToolBar();
   //UISetCheck(ID_VIEW_TOOLBAR, IsToolbarVisible());
   UISetCheck(ID_VIEW_STATUS_BAR, ::IsWindowVisible(m_hWndStatusBar));
   m_dockingWindowMenu.UpdateMenu();
   return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
