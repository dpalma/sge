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
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
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
   CRect rect;
   GetClientRect(rect);
   CSize treeSize(min(kDefaultTreeWidth, rect.Width() / 4),0);
   return m_wndSplitter.CreateStatic(this, 1, 2)
      && m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CTreeView), treeSize, pContext)
      && m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(cEditorView), CSize(0,0), pContext);
}
