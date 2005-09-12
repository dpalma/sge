/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorApp.h"
#include "editorDoc.h"
#include "editorView.h"
#include "editorTypes.h"
#include "MainFrm.h"
#include "aboutdlg.h"
#include "BitmapUtils.h"
#include "terrainapi.h"
#include "ScriptCmdDlg.h"

#include "inputapi.h"
#include "scriptapi.h"
#include "engineapi.h"
#include "saveloadapi.h"
#include "sys.h"

#include "resourceapi.h"
#include "configapi.h"
#include "techtime.h"
#include "connptimpl.h"
#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "threadcallapi.h"

#include <algorithm>

#ifdef HAVE_CPPUNIT
#ifdef USE_MFC_TESTRUNNER
#include <cppunit/ui/mfc/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern sScriptReg g_editorCmds[];
extern uint g_nEditorCmds;

static const tChar g_szRegistryKey[] = _T("SGE");

/////////////////////////////////////////////////////////////////////////////

WTL::CAppModule _Module;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorApp
//

BEGIN_MESSAGE_MAP(cEditorApp, CWinApp)
	//{{AFX_MSG_MAP(cEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_TOOLS_UNITTESTRUNNER, OnToolsUnitTestRunner)
	ON_COMMAND(ID_TOOLS_EXECUTESCRIPTCOMMAND, OnToolsExecuteScriptCommand)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorApp construction

cEditorApp::cEditorApp()
 : m_hCurrentToolWnd(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
   RegisterGlobalObject(IID_IEditorApp, static_cast<IGlobalObject*>(this));
}

///////////////////////////////////////

cEditorApp::~cEditorApp()
{
}

///////////////////////////////////////

tResult cEditorApp::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cEditorApp::Term()
{
   return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// The one and only cEditorApp object

cEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// cEditorApp initialization

static bool ScriptExecFile(const char * pszFile)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecFile(pszFile) == S_OK;
}

////////////////////////////////////////

static bool ScriptExecString(const char * pszCode)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecString(pszCode) == S_OK;
}

////////////////////////////////////////

static void ScriptCallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   UseGlobal(ScriptInterpreter);
   va_list args;
   va_start(args, pszArgDesc);
   pScriptInterpreter->CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

////////////////////////////////////////

static char * GetEntireContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   ulong length;
   pReader->Tell(&length);
   pReader->Seek(0, kSO_Set);

   char * pszContents = new char[length + 1];

   if (pReader->Read(pszContents, length) != S_OK)
   {
      delete [] pszContents;
      return NULL;
   }

   pszContents[length] = 0;

   return pszContents;
}

////////////////////////////////////////

static bool ScriptExecResource(const char * pszResource)
{
   bool bResult = false;
   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(tResKey(pszResource, kRC_Text), (void**)&pszCode) == S_OK)
   {
      bResult = ScriptExecString(pszCode);
      pResourceManager->Unload(tResKey(pszResource, kRC_Text));
   }
   return bResult;
}

////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
//   SimCreate();
   ResourceManagerCreate();
   ScriptInterpreterCreate();
//   GUIContextCreate();
//   GUIFactoryCreate();
   EditorTileSetsCreate();
   ThreadCallerCreate();
   TerrainModelCreate();
   TerrainRendererCreate(true);
   SaveLoadManagerCreate();
}

////////////////////////////////////////

BOOL cEditorApp::InitInstance()
{
   if (FAILED(_Module.Init(NULL, AfxGetInstanceHandle())))
   {
      ErrorMsg("Error initializing ATL module object\n");
      return FALSE;
   }

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
	SetRegistryKey(g_szRegistryKey);

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return FALSE;
   }

   TargaFormatRegister();
   BmpFormatRegister();
   TextFormatRegister("txt");
   TextFormatRegister("lua");
   TextFormatRegister("xml");
   EngineRegisterResourceFormats();
   BitmapUtilsRegisterResourceFormats();
   RegisterTerrainResourceFormats();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadInit();

   ScriptAddFunctions(g_editorCmds, g_nEditorCmds);

   cFileSpec file(__argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(file);
   if (pStore->Load(g_pConfig) != S_OK)
   {
      DebugMsg1("Error loading settings from %s\n", file.c_str());
   }

   ::ParseCommandLine(__argc, __argv, g_pConfig);

   cStr temp;
   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      // Attempt to load as archive
      if (FAILED(pResourceManager->AddArchive(temp.c_str())))
      {
         if (FAILED(pResourceManager->AddDirectoryTreeFlattened(temp.c_str())))
         {
            ErrorMsg1("Unable to set up resource directory %s\n", temp.c_str());
            return FALSE;
         }
      }
   }

   cAutoIPtr<IEditorSplashScreen> pEditorSplashScreen;
   EditorSplashScreenCreate(&pEditorSplashScreen);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

   cEditorSingleDocTemplate * pDocTemplate = new cEditorSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cEditorView));
	AddDocTemplate(pDocTemplate);

   cStr autoexecScript("editor.lua");
   ConfigGet("editor_autoexec_script", &autoexecScript);

   if (!ScriptExecFile(autoexecScript.c_str()))
   {
      if (!ScriptExecResource(autoexecScript.c_str()))
      {
         WarnMsg1("Error parsing or executing %s\n", autoexecScript.c_str());
      }
   }

   ScriptCallFunction("EditorInit", NULL);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
//	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

   CFrameWnd * pMainFrame = DYNAMIC_DOWNCAST(CFrameWnd, m_pMainWnd);
   if (pMainFrame == NULL)
   {
      return FALSE;
   }
   cEditorView * pEditorView = DYNAMIC_DOWNCAST(cEditorView, pMainFrame->GetActiveView());
   if (!pEditorView->Initialize())
   {
      return FALSE;
   }

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

////////////////////////////////////////

int cEditorApp::ExitInstance() 
{
	StopGlobalObjects();

   _Module.Term();

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// cEditorApp message handlers

// App command to run the dialog
void cEditorApp::OnAppAbout()
{
	CAboutDlg().DoModal();
}

////////////////////////////////////////

int cEditorApp::Run() 
{
   ASSERT_VALID(this);

   // for tracking the idle time state
   bool bIdle = true;
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
            bIdle = false; // assume "no idle" state
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
            bIdle = true;
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

   Assert(!"Should never reach this point!"); // not reachable
}

////////////////////////////////////////

BOOL cEditorApp::PreTranslateMessage(MSG * pMsg) 
{
   cAutoIPtr<IEditorTool> pTool;
   if (AccessToolCapture() != NULL)
   {
      pTool = CTAddRef(AccessToolCapture());
   }
   else if (AccessActiveTool() != NULL)
   {
      pTool = CTAddRef(AccessActiveTool());
   }
   //else if (AccessDefaultTool() != NULL)
   //{
   //   pTool = CTAddRef(AccessDefaultTool());
   //}

   if (!!pTool)
   {
      m_hCurrentToolWnd = pMsg->hwnd;

      Assert(!m_pCurrentToolView);
      CWnd * pWnd = CWnd::FromHandlePermanent(pMsg->hwnd);
      if (pWnd != NULL)
      {
         cEditorView * pEditorView = DYNAMIC_DOWNCAST(cEditorView, pWnd);
         if (pEditorView != NULL)
         {
            m_pCurrentToolView = CTAddRef(static_cast<IEditorView *>(pEditorView));
         }
      }

      tResult toolResult = S_EDITOR_TOOL_CONTINUE;

      switch (pMsg->message)
      {
         case WM_KEYDOWN:
         {
            toolResult = pTool->OnKeyDown(cEditorKeyEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);

            if ((toolResult == S_EDITOR_TOOL_CONTINUE) && (pMsg->wParam == VK_ESCAPE))
            {
               SetActiveTool(NULL);
            }
            break;
         }

         case WM_KEYUP:
         {
            toolResult = pTool->OnKeyUp(cEditorKeyEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONDBLCLK:
         {
            toolResult = pTool->OnLButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONDOWN:
         {
            toolResult = pTool->OnLButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_LBUTTONUP:
         {
            toolResult = pTool->OnLButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONDBLCLK:
         {
            toolResult = pTool->OnRButtonDblClk(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONDOWN:
         {
            toolResult = pTool->OnRButtonDown(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_RBUTTONUP:
         {
            toolResult = pTool->OnRButtonUp(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_MOUSEMOVE:
         {
            toolResult = pTool->OnMouseMove(cEditorMouseEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }

         case WM_MOUSEWHEEL:
         {
            toolResult = pTool->OnMouseWheel(cEditorMouseWheelEvent(pMsg->wParam, pMsg->lParam), m_pCurrentToolView);
            break;
         }
      }

      m_hCurrentToolWnd = NULL;
      SafeRelease(m_pCurrentToolView);

      if (toolResult == S_EDITOR_TOOL_HANDLED)
      {
         return TRUE;
      }
   }

   return CWinApp::PreTranslateMessage(pMsg);
}

////////////////////////////////////////

tResult cEditorApp::AddLoopClient(IEditorLoopClient * pLoopClient)
{
   return add_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::RemoveLoopClient(IEditorLoopClient * pLoopClient)
{
   return remove_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::AddEditorAppListener(IEditorAppListener * pListener)
{
   return add_interface(m_editorAppListeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::RemoveEditorAppListener(IEditorAppListener * pListener)
{
   return remove_interface(m_editorAppListeners, pListener) ? S_OK : E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::GetActiveView(IEditorView * * ppView)
{
   if (ppView == NULL)
   {
      return E_POINTER;
   }

   CFrameWnd * pMainFrm = DYNAMIC_DOWNCAST(CFrameWnd, GetMainWnd());
   if (pMainFrm != NULL)
   {
      cEditorView * pEditorView = DYNAMIC_DOWNCAST(cEditorView, pMainFrm->GetActiveView());
      if (pEditorView != NULL)
      {
         *ppView = CTAddRef(static_cast<IEditorView *>(pEditorView));
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::GetActiveModel(IEditorModel * * ppModel)
{
   if (ppModel == NULL)
   {
      return E_POINTER;
   }

   CFrameWnd * pMainFrm = DYNAMIC_DOWNCAST(CFrameWnd, GetMainWnd());
   if (pMainFrm != NULL)
   {
      cEditorDoc * pEditorDoc = DYNAMIC_DOWNCAST(cEditorDoc, pMainFrm->GetActiveDocument());
      if (pEditorDoc != NULL)
      {
         *ppModel = CTAddRef(static_cast<IEditorModel *>(pEditorDoc));
         return S_OK;
      }
   }

   return E_FAIL;
}

////////////////////////////////////////

tResult cEditorApp::GetActiveTool(IEditorTool * * ppTool)
{
   return m_pActiveTool.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorApp::SetActiveTool(IEditorTool * pTool)
{
   tEditorAppListeners::iterator iter;
   for (iter = m_editorAppListeners.begin(); iter != m_editorAppListeners.end(); iter++)
   {
      (*iter)->OnActiveToolChange(pTool, m_pActiveTool);
   }

   if (!!m_pActiveTool)
   {
      m_pActiveTool->Deactivate();
   }

   SafeRelease(m_pActiveTool);

   if (pTool != NULL)
   {
      pTool->Activate();
      m_pActiveTool = CTAddRef(pTool);
   }

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::GetToolCapture(IEditorTool * * ppTool)
{
   return m_pToolCapture.GetPointer(ppTool);
}

////////////////////////////////////////

tResult cEditorApp::SetToolCapture(IEditorTool * pTool)
{
   if (pTool == NULL)
   {
      return E_POINTER;
   }

   if (!IsWindow(m_hCurrentToolWnd))
   {
      DebugMsg("No valid window handle in SetToolCapture\n");
      return E_FAIL;
   }

   ::SetCapture(m_hCurrentToolWnd);

   SafeRelease(m_pToolCapture);

   m_pToolCapture = CTAddRef(pTool);

   return S_OK;
}

////////////////////////////////////////

tResult cEditorApp::ReleaseToolCapture()
{
   if (!m_pToolCapture)
   {
      return S_FALSE;
   }
   else
   {
      ReleaseCapture();
      SafeRelease(m_pToolCapture);
      return S_OK;
   }
}

////////////////////////////////////////

void cEditorApp::OnToolsUnitTestRunner() 
{
#ifdef HAVE_CPPUNIT
#ifdef USE_MFC_TESTRUNNER
   CppUnit::MfcUi::TestRunner runner;
   runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
   runner.run();
#else
   SysRunUnitTests();
#endif
#else
   AfxMessageBox(IDS_NO_UNIT_TESTS);
#endif
}

////////////////////////////////////////

void cEditorApp::OnToolsExecuteScriptCommand()
{
   cScriptCmdDlg dlg(GetMainWnd());
   if (dlg.DoModal() == IDOK)
   {
      UseGlobal(ScriptInterpreter);
      if (FAILED(pScriptInterpreter->ExecString(dlg.m_scriptCommand)))
      {
         MessageBeep(MB_ICONSTOP);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cEditorAppTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cEditorAppTests);
   CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(cEditorAppTests);

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
