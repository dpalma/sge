/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "MainFrm.h"

#include "editorView.h"
#include "editorCtrlBars.h"

#include <afxcview.h>

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int kDefaultTreeWidth = 250;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_CONTROL_BAR1, OnUpdateViewControlBarMenu)
	//}}AFX_MSG_MAP
	ON_COMMAND_EX_RANGE(ID_VIEW_CONTROL_BAR1, ID_VIEW_CONTROL_BAR16, OnViewControlBar)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
   std::vector<CControlBar *>::iterator iter;
   for (iter = m_ctrlBars.begin(); iter != m_ctrlBars.end(); iter++)
   {
      delete *iter;
   }
   m_ctrlBars.clear();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

   static const uint ctrlBarPlacementMap[] =
   {
      AFX_IDW_DOCKBAR_TOP, // kCBP_Top
      AFX_IDW_DOCKBAR_LEFT, // kCBP_Left
      AFX_IDW_DOCKBAR_RIGHT, //kCBP_Right
      AFX_IDW_DOCKBAR_BOTTOM, //kCBP_Bottom
      AFX_IDW_DOCKBAR_FLOAT, //kCBP_Float
   };

   uint titleStringId;
   CRuntimeClass * pRuntimeClass;
   eControlBarPlacement placement;

   uint ctrlBarId = AFX_IDW_CONTROLBAR_FIRST + 32;

   const CRect rect(0,0,0,0);

   static const DWORD ctrlBarStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN;

   const CString ctrlBarWndClass = AfxRegisterWndClass(CS_DBLCLKS,
      LoadCursor(NULL, IDC_ARROW), GetSysColorBrush(COLOR_BTNFACE), 0);

   std::vector<uint> dockBars;
   HANDLE hIter;
   IterCtrlBarsBegin(&hIter);
   while (IterNextCtrlBar(&hIter, &titleStringId, &pRuntimeClass, &placement))
   {
      CString title;
      if (!title.LoadString(titleStringId))
      {
         title.Format("ControlBar%d", titleStringId);
      }

      if (pRuntimeClass != NULL && pRuntimeClass->m_pfnCreateObject != NULL)
      {
         CControlBar * pCtrlBar = DYNAMIC_DOWNCAST(CControlBar, pRuntimeClass->CreateObject());
         if (pCtrlBar != NULL)
         {
            if (pCtrlBar->Create(ctrlBarWndClass, title, ctrlBarStyle, rect, this, ctrlBarId))
            {
               pCtrlBar->EnableDocking(CBRS_ALIGN_ANY);
               pCtrlBar->SetBarStyle(pCtrlBar->GetBarStyle() |
                  CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

               ctrlBarId++;
               m_ctrlBars.push_back(pCtrlBar);
               dockBars.push_back(ctrlBarPlacementMap[placement]);
            }
            else
            {
               DebugMsg1("Error creating control bar of type %s\n", pRuntimeClass->m_lpszClassName);
               delete pCtrlBar;
            }
         }
      }
   }
   IterCtrlBarsEnd(hIter);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

   Assert(m_ctrlBars.size() == dockBars.size());

   for (uint i = 0; i < m_ctrlBars.size(); i++)
   {
	   DockControlBar(m_ctrlBars[i], dockBars[i]);
   }

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
   for (uint i = 0; i < m_ctrlBars.size(); i++)
   {
      ASSERT_VALID(m_ctrlBars[i]);
   }
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
#if 0
   CRect rect;
   GetClientRect(rect);
   CSize treeSize(min(kDefaultTreeWidth, rect.Width() / 4),0);
   return m_wndSplitter.CreateStatic(this, 1, 2)
      && m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CTreeView), treeSize, pContext)
      && m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(cEditorView), CSize(0,0), pContext);
#else
   return CFrameWnd::OnCreateClient(lpcs, pContext);
#endif
}

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

BOOL CMainFrame::OnViewControlBar(UINT nID)
{
   ASSERT_VALID(this);
   Assert(nID >= ID_VIEW_CONTROL_BAR1);
   Assert(nID < (ID_VIEW_CONTROL_BAR1 + m_ctrlBars.size()));
   int index = nID - ID_VIEW_CONTROL_BAR1;
   ASSERT_VALID(m_ctrlBars[index]);
   CControlBar * pCtrlBar = m_ctrlBars[index];
   ShowControlBar(pCtrlBar, !pCtrlBar->IsWindowVisible(), FALSE);
   return TRUE;
}
