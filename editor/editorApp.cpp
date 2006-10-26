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
#include "engine/terrainapi.h"
#include "ScriptCmdDlg.h"

#include "engine/cameraapi.h"
#include "engine/engineapi.h"
#include "engine/entityapi.h"
#include "platform/inputapi.h"
#include "render/renderapi.h"
#include "engine/saveloadapi.h"
#include "tech/schedulerapi.h"
#include "script/scriptapi.h"
#include "platform/sys.h"

#include "tech/resourceapi.h"
#include "tech/configapi.h"
#include "tech/techtime.h"
#include "tech/connptimpl.h"
#include "tech/readwriteapi.h"
#include "tech/filespec.h"
#include "tech/filepath.h"
#include "tech/threadcallapi.h"
#include "tech/imageapi.h"

#include <algorithm>

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

extern sScriptReg g_editorCmds[];
extern uint g_nEditorCmds;

extern tResult EditorToolStateCreate();

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
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
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

static bool ScriptExecResource(const char * pszResource)
{
   bool bResult = false;
   char * pszCode = NULL;
   UseGlobal(ResourceManager);
   if (pResourceManager->Load(pszResource, kRT_Text, NULL, (void**)&pszCode) == S_OK)
   {
      bResult = ScriptExecString(pszCode);
   }
   return bResult;
}

////////////////////////////////////////

static void RegisterGlobalObjects()
{
   CameraCreate();
   CameraControlCreate();
   EditorToolStateCreate();
   EntityManagerCreate();
   InputCreate();
   ResourceManagerCreate();
   SaveLoadManagerCreate();
   SchedulerCreate();
   ScriptInterpreterCreate();
   RendererCreate();
   TerrainModelCreate();
   TerrainRendererCreateForEditor();
   ThreadCallerCreate();
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

   RegisterGlobalObject(IID_IEditorApp, static_cast<IGlobalObject*>(this));
   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return FALSE;
   }

   TextFormatRegister(_T("css,lua,xml"));
   EngineRegisterResourceFormats();
   TerrainRegisterResourceFormats();
   ImageRegisterResourceFormats();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadInit();

   ScriptAddFunctions(g_editorCmds, g_nEditorCmds);

   cFileSpec file(__argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(file);
   if (pStore->Load(g_pConfig) != S_OK)
   {
      DebugMsg1("Error loading settings from %s\n", file.CStr());
   }

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

   cEditorSingleDocTemplate * pDocTemplate = new cEditorSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cEditorView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

   cAutoIPtr<IEditorSplashScreen> pEditorSplashScreen;
   if (cmdInfo.m_bShowSplash)
   {
      EditorSplashScreenCreate(&pEditorSplashScreen);
   }

   cStr temp;
   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      // Attempt to load as archive
      if (pResourceManager->AddArchive(temp.c_str()) != S_OK)
      {
         if (pResourceManager->AddDirectoryTreeFlattened(temp.c_str()) != S_OK)
         {
            ErrorMsg1("Unable to set up resource directory %s\n", temp.c_str());
            return FALSE;
         }
      }
   }

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

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
   {
		return FALSE;
   }

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

   g_pConfig->Clear();

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

   UseGlobal(Scheduler);
   pScheduler->Start();

   MSG lastMouseMove = {0};

   // acquire and dispatch messages until a WM_QUIT message is received.
   for (;;)
   {
#if _MFC_VER >= 0x0700
	   _AFX_THREAD_STATE * pState = AfxGetThreadState();
      MSG * pMsg = &(pState->m_msgCur);
#else
      MSG * pMsg = &m_msgCur;
#endif

      // phase1: check to see if we can do idle work
      while (bIdle &&
         !::PeekMessage(pMsg, NULL, 0, 0, PM_NOREMOVE))
      {
         // call OnIdle while in bIdle state
         if (!OnIdle(lIdleCount++))
            bIdle = false; // assume "no idle" state
      }

      // phase2: pump messages while available
      while (::PeekMessage(pMsg, NULL, 0, 0, PM_NOREMOVE))
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

      pScheduler->NextFrame();
   }

   Assert(!"Should never reach this point!"); // not reachable
}

////////////////////////////////////////

BOOL cEditorApp::PreTranslateMessage(MSG * pMsg) 
{
   cAutoIPtr<IEditorView> pEditorView;
   CWnd * pWnd = CWnd::FromHandlePermanent(pMsg->hwnd);
   if (pWnd != NULL)
   {
      cEditorView * pEditorViewWnd = DYNAMIC_DOWNCAST(cEditorView, pWnd);
      if (pEditorViewWnd != NULL)
      {
         pEditorView = CTAddRef(static_cast<IEditorView*>(pEditorViewWnd));
      }
   }

   if (!!pEditorView)
   {
      UseGlobal(EditorToolState);
      if (pEditorToolState->HandleMessage(pMsg) == S_OK)
      {
         return TRUE;
      }
   }

   return CWinApp::PreTranslateMessage(pMsg);
}

////////////////////////////////////////

void cEditorApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
   for (int i = 1; i < __argc; i++)
   {
      const tChar * pszParam = __targv[i];
      if (pszParam[0] == '-')
      {
         g_pConfig->Delete(++pszParam);
      }
      else if (pszParam[0] == '+')
      {
         ++pszParam;
         const tChar * pszEq = _tcschr(pszParam, _T('='));
         if (pszEq != NULL)
         {
            CString key(pszParam, pszEq - pszParam);
            key.Trim();
            CString value(++pszEq);
            value.Trim();
            g_pConfig->Set(key, value);
         }
         else
         {
            g_pConfig->Set(pszParam, "1");
         }
      }
      else
      {
         BOOL bFlag = FALSE;
         BOOL bLast = ((i + 1) == __argc);
         if (pszParam[0] == '/')
         {
            // remove flag specifier
            bFlag = TRUE;
            ++pszParam;
         }
         rCmdInfo.ParseParam(pszParam, bFlag, bLast);
      }
   }
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

///////////////////////////////////////

tResult cEditorApp::SetDefaultTileSet(const tChar * pszTileSet)
{
   if (pszTileSet != NULL)
   {
      if (m_defaultTileSet.compare(pszTileSet) == 0)
      {
         return S_FALSE;
      }
      else
      {
         m_defaultTileSet = pszTileSet;
         tEditorAppListeners::iterator iter = m_editorAppListeners.begin();
         for (; iter != m_editorAppListeners.end(); iter++)
         {
            (*iter)->OnDefaultTileSetChange(pszTileSet);
         }
      }
   }
   else
   {
      m_defaultTileSet.erase();
      tEditorAppListeners::iterator iter = m_editorAppListeners.begin();
      for (; iter != m_editorAppListeners.end(); iter++)
      {
         (*iter)->OnDefaultTileSetChange(NULL);
      }
   }

   return S_OK;
}

///////////////////////////////////////

tResult cEditorApp::GetDefaultTileSet(cStr * pTileSet) const
{
   if (pTileSet == NULL)
   {
      return E_POINTER;
   }
   *pTileSet = m_defaultTileSet;
   return m_defaultTileSet.empty() ? S_FALSE : S_OK;
}

////////////////////////////////////////

void cEditorApp::OnToolsUnitTestRunner() 
{
#if defined(HAVE_CPPUNIT) || defined(HAVE_UNITTESTPP)
   SysRunUnitTests();
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
