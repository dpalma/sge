/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "igApp.h"
#include "MainFrm.h"

#include "tech/configapi.h"
#include "tech/dictionaryapi.h"
#include "tech/filepath.h"
#include "tech/filespec.h"
#include "tech/globalobj.h"
#include "tech/imageapi.h"
#include "tech/resourceapi.h"

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////

WTL::CAppModule _Module;

/////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   ResourceManagerCreate();
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
	if (!::InitCommonControlsEx(&iccx))
   {
      return -1;
   }

	if (FAILED(_Module.Init(NULL, hInstance)))
   {
      return -1;
   }

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

   ParseCommandLine(__argc, __argv, g_pConfig);

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

   WTL::CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame mainWindow;
	if (!mainWindow.CreateEx())
	{
		ErrorMsg("Main window creation failed\n");
		return -1;
	}

	mainWindow.ShowWindow(nCmdShow);

	int result = theLoop.Run();

	_Module.RemoveMessageLoop();

   StopGlobalObjects();

	_Module.Term();

	return result;
}

///////////////////////////////////////////////////////////////////////////////
