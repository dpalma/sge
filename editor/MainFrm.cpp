/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MainFrm.h"

#include "editorView.h"
#include "editorCtrlBars.h"
#include "aboutdlg.h"
#include "MapSettingsDlg.h"
#include "editorTypes.h"

#include "globalobj.h"

#include <DockMisc.h>
#include <dwstate.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

//////////////////////////////////////////////////////////////////////////////

static const SIZE g_mapSizes[] =
{
   { 64, 64 },
   { 128, 128 },
   { 192, 192 },
   { 256, 256 },
};

static const uint kDefaultMapSizeIndex = 0;

//////////////////////////////////////////////////////////////////////////////

template <typename CONTAINER>
void ListTileSets(CONTAINER * pContainer)
{
   UseGlobal(EditorTileManager);

   uint nTileSets = 0;
   if (pEditorTileManager->GetTileSetCount(&nTileSets) == S_OK && nTileSets > 0)
   {
      for (uint i = 0; i < nTileSets; i++)
      {
         cAutoIPtr<IEditorTileSet> pTileSet;
         if (pEditorTileManager->GetTileSet(i, &pTileSet) == S_OK)
         {
            Assert(!!pTileSet);
            cStr name;
            Verify(pTileSet->GetName(&name) == S_OK);
            pContainer->push_back(name);
         }
         else
         {
            WarnMsg1("Error getting tile set %d\n", i);
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// double up any '&' characters so they do not get underlined in menus

static int FixAmpersands(const tChar * pszIn, CString * pOut)
{
   Assert(pszIn && pOut);

   int nAmpersands = 0;

   const tChar * p = pszIn;
   while (*p++)
   {
      if (*p == '&')
      {
         nAmpersands++;
      }
   }

   tChar * pszTemp = static_cast<tChar *>(alloca(strlen(pszIn) + nAmpersands + 1));
   while (*pszIn)
   {
      if (*pszIn == '&')
      {
         // Add an extra ampersand for every one found
         *pszTemp++ = '&';
      }

      if (_istlead(*pszIn))
      {
         *pszTemp++ = *pszIn++;
      }

      *pszTemp++ = *pszIn++;
   }

   *pszTemp = 0;
   *pOut = pszTemp;

   return nAmpersands;
}

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

         FixAmpersands(title, &title);

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
//
// CLASS: cMainFrame
//

////////////////////////////////////////

cMainFrame::cMainFrame()
 : m_bPromptForMapSettings(false),
   m_dockingWindowMenu(m_dockingWindows, ID_VIEW_DOCKING_WINDOW_FIRST, ID_VIEW_DOCKING_WINDOW_LAST)
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

   Assert(!m_pMainView);
   Assert(m_hWndClient == NULL);
   if (CoCreateInstance(CLSID_EditorView, NULL, CLSCTX_ALL, IID_IEditorView, (void**)&m_pMainView) != S_OK
      || m_pMainView->Create(m_hWnd, &m_hWndClient) != S_OK)
   {
      ErrorMsg("Error creating main view\n");
      return -1;
   }

   CMessageLoop * pMessageLoop = _Module.GetMessageLoop();
   pMessageLoop->AddIdleHandler(this);
   pMessageLoop->AddMessageFilter(this);

   InitializeDockingFrame();

   CreateDockingWindows();

   // App starts of with a File->New command
   SendMessage(WM_COMMAND, ID_FILE_NEW);

   // Once the app starts, every File->New should prompt for map settings
   m_bPromptForMapSettings = true;

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
   if (!!m_pMainView)
   {
      m_pMainView->Destroy();
      SafeRelease(m_pMainView);
   }

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
   CComPtr<IEditorModel> pModel;
   if (CoCreateInstance(CLSID_EditorDoc, NULL, CLSCTX_ALL, IID_IEditorModel, (void**)&pModel) != S_OK)
   {
      ErrorMsg("Failed to create document object\n");
      return -1;
   }

   std::vector<cStr> tileSets;
   ListTileSets(&tileSets);

   cMapSettings mapSettings(
      g_mapSizes[kDefaultMapSizeIndex].cx,
      g_mapSizes[kDefaultMapSizeIndex].cy,
      tileSets.empty() ? "" : tileSets[0]);

   if (m_bPromptForMapSettings)
   {
      cMapSettingsDlg dlg(g_mapSizes, _countof(g_mapSizes), kDefaultMapSizeIndex,
         tileSets, 0, kHeightData_None);

      // Shouldn't be allowed to cancel the dialog
      Verify(dlg.DoModal(m_hWnd) == IDOK);

      SIZE mapSize;
      cStr tileSet, heightMap;

      Verify(dlg.GetSelectedSize(&mapSize));
      Verify(dlg.GetSelectedTileSet(&tileSet));
      Verify(dlg.GetHeightDataFile(&heightMap));

      mapSettings = cMapSettings(
         mapSize.cx,
         mapSize.cy,
         tileSet,
         dlg.GetHeightData(),
         heightMap.empty() ? NULL : heightMap.c_str());
   }

   Verify(SetModel(pModel) == S_OK);

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

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cMainFrameTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cMainFrameTests);
      CPPUNIT_TEST(TestFixAmpersands);
   CPPUNIT_TEST_SUITE_END();

   void TestFixAmpersands();
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cMainFrameTests);

////////////////////////////////////////

void cMainFrameTests::TestFixAmpersands()
{
   static const struct
   {
      const tChar * pszTest;
      const tChar * pszExpected;
   }
   tests[] =
   {
      { "", "" },
      { "test", "test" },
      { "te&st", "te&&st" },
   };
   for (int i = 0; i < _countof(tests); i++)
   {
      CString out;
      FixAmpersands(tests[i].pszTest, &out);
      CPPUNIT_ASSERT(out.Compare(tests[i].pszExpected) == 0);
   }
}

#endif // HAVE_CPPUNIT

/////////////////////////////////////////////////////////////////////////////
