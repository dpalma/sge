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
#include "scriptapi.h"

#include "textureapi.h"

#include "resmgr.h"
#include "configapi.h"
#include "globalobj.h"
#include "techtime.h"
#include "connptimpl.h"
#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "str.h"

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

static bool ScriptExecFile(const char * pszFile)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecFile(pszFile) == S_OK;
}

static bool ScriptExecString(const char * pszCode)
{
   UseGlobal(ScriptInterpreter);
   return pScriptInterpreter->ExecString(pszCode) == S_OK;
}

static void ScriptCallFunction(const char * pszName, const char * pszArgDesc, ...)
{
   UseGlobal(ScriptInterpreter);
   va_list args;
   va_start(args, pszArgDesc);
   pScriptInterpreter->CallFunction(pszName, pszArgDesc, args);
   va_end(args);
}

static char * GetEntireContents(IReader * pReader)
{
   Assert(pReader != NULL);

   pReader->Seek(0, kSO_End);
   int length = pReader->Tell();
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

static bool ScriptExecResource(const char * pszResource)
{
   UseGlobal(ResourceManager);
   cAutoIPtr<IReader> pReader = pResourceManager->Find(pszResource);
   if (!pReader)
      return false;
   char * pszCode = GetEntireContents(pReader);
   if (pszCode == NULL)
      return false;
   bool result = ScriptExecString(pszCode);
   delete [] pszCode;
   return result;
}

static void RegisterGlobalObjects()
{
   InputCreate();
//   SimCreate();
   ResourceManagerCreate();
   SceneCreate();
   ScriptInterpreterCreate();
   TextureManagerCreate();
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
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return FALSE;
   }

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate * pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cEditorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cEditorView));
	AddDocTemplate(pDocTemplate);

   cFileSpec file(__argv[0]);
   file.SetPath(cFilePath());
   file.SetFileExt("cfg");

   cAutoIPtr<IConfigStore> pConfigStore = CreateTextConfigStore(file);
   pConfigStore->Load(g_pConfig);

   g_pConfig->ParseCmdLine(__argc, __argv);

   cStr temp;
   if (ConfigGet("data", &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddSearchPath(temp);
   }

   cStr autoExecScript("editor.lua");
   ConfigGet("editor_autoexec_script", &autoExecScript);

   if (!ScriptExecFile(autoExecScript))
   {
      if (!ScriptExecResource(autoExecScript))
      {
         DebugMsg1("Error parsing or executing %s\n", autoExecScript.c_str());
         return FALSE;
      }
   }

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
	CAboutDlg().DoModal();
}

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

tResult cEditorApp::AddLoopClient(IEditorLoopClient * pLoopClient)
{
   return add_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}

tResult cEditorApp::RemoveLoopClient(IEditorLoopClient * pLoopClient)
{
   return remove_interface(m_loopClients, pLoopClient) ? S_OK : E_FAIL;
}
