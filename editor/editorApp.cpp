/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorApp.h"
#include "editorDoc.h"
#include "editorView.h"
#include "MainFrm.h"
#include "aboutdlg.h"

#include "sceneapi.h"
#include "inputapi.h"

#include "resmgr.h"
#include "globalobj.h"
#include "techtime.h"
#include "connptimpl.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cEditorApp

BEGIN_MESSAGE_MAP(cEditorApp, CWinApp)
	//{{AFX_MSG_MAP(cEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorApp construction

cEditorApp::cEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only cEditorApp object

cEditorApp theApp;

IEditorApp * AccessEditorApp()
{
   return static_cast<IEditorApp *>(&theApp);
}

/////////////////////////////////////////////////////////////////////////////
// cEditorApp initialization

static void RegisterGlobalObjects()
{
   InputCreate();
//   SimCreate();
   ResourceManagerCreate();
   SceneCreate();
//   ScriptInterpreterCreate();
//   TextureManagerCreate();
//   GUIContextCreate();
//   GUIFactoryCreate();
//   GUIRenderingToolsCreate();
}

BOOL cEditorApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

   RegisterGlobalObjects();
   StartGlobalObjects();

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate * pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cEditorView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

int cEditorApp::ExitInstance() 
{
	StopGlobalObjects();
	
	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// cEditorApp message handlers

// App command to run the dialog
void cEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

int cEditorApp::Run() 
{
   ASSERT_VALID(this);

   // for tracking the idle time state
   BOOL bIdle = TRUE;
   long lIdleCount = 0;

   double timeLastFrame = TimeGetSecs();
   double time = timeLastFrame;

   // acquire and dispatch messages until a WM_QUIT message is received.
   for (;;)
   {
      // phase1: check to see if we can do idle work
      double timeEnterIdle = TimeGetSecs();
      while (bIdle &&
         (TimeGetSecs() - timeEnterIdle) < 0.05)
//         !::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE))
      {
         // call OnIdle while in bIdle state
         if (!OnIdle(lIdleCount++))
            bIdle = FALSE; // assume "no idle" state
      }

#if _MFC_VER >= 0x0700
	   _AFX_THREAD_STATE * pState = AfxGetThreadState();
      MSG * pMsg = &(pState->m_msgCur);
#else
      MSG * pMsg = &m_msgCur;
#endif

      // phase2: pump messages while available
      while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
      {
         // pump message, but quit on WM_QUIT
         if (!PumpMessage())
            return ExitInstance();

         // reset "no idle" state after pumping "normal" message
         if (IsIdleMessage(pMsg))
         {
            bIdle = TRUE;
            lIdleCount = 0;
         }
      }

      double elapsed = time - timeLastFrame;

      tEditorLoopClients::iterator iter;
      for (iter = m_loopClients.begin(); iter != m_loopClients.end(); iter++)
      {
         (*iter)->OnFrame(time, elapsed);
      }

      timeLastFrame = time;
      time = TimeGetSecs();
   }

   Assert(!"Should never reach this point!");  // not reachable
}

tResult cEditorApp::AddLoopClient(IEditorLoopClient * pLoopClient)
{
   return add_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

tResult cEditorApp::RemoveLoopClient(IEditorLoopClient * pLoopClient)
{
   return remove_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}
