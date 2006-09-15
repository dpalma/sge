/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igApp.h"
#include "igDoc.h"
#include "igView.h"
#include "MainFrm.h"

#include "configapi.h"
#include "dictionaryapi.h"
#include "filepath.h"
#include "filespec.h"
#include "globalobj.h"
#include "imageapi.h"
#include "resourceapi.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImageGenApp
//

BEGIN_MESSAGE_MAP(cImageGenApp, CWinApp)
	//{{AFX_MSG_MAP(cImageGenApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cImageGenApp construction

cImageGenApp::cImageGenApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

///////////////////////////////////////

cImageGenApp::~cImageGenApp()
{
}


/////////////////////////////////////////////////////////////////////////////
// The one and only cImageGenApp object

cImageGenApp theApp;

/////////////////////////////////////////////////////////////////////////////
// cImageGenApp initialization

////////////////////////////////////////

static void RegisterGlobalObjects()
{
   ResourceManagerCreate();
}

////////////////////////////////////////

BOOL cImageGenApp::InitInstance()
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
	SetRegistryKey(_T("ImageGen"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      ErrorMsg("One or more application-level services failed to start!\n");
      return FALSE;
   }

   TextFormatRegister(_T("xml,txt,cfg"));
   ImageRegisterResourceFormats();

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

   CSingleDocTemplate * pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(cImageGenDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(cImageGenView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

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

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
   {
		return FALSE;
   }

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

////////////////////////////////////////

int cImageGenApp::ExitInstance() 
{
	StopGlobalObjects();

   g_pConfig->Clear();

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// cImageGenApp message handlers

// App command to run the dialog
void cImageGenApp::OnAppAbout()
{
	CDialog(IDD_ABOUTBOX, AfxGetMainWnd()).DoModal();
}

////////////////////////////////////////

void cImageGenApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
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

///////////////////////////////////////////////////////////////////////////////
